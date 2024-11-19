// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "ItemSystemCore.h"
#include "WeaponActor.h"
#include "Ammo/AmmoObject.h"
#include "Character/CharacterInventoryComponent.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UWeaponInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, WeaponData, COND_OwnerOnly);
}

bool UWeaponInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (UAmmoObject* Round : WeaponData.Rounds)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(Round, *Bunch, *RepFlags);
		bReplicateSomething |= Round->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
}

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                      const UItemPredictedData* PredictedData)
{
	Super::SetupProperties(NewItemId, Definition, PredictedData);

	if (auto WeaponDefinition = Cast<UWeaponDefinition>(Definition))
	{
		WeaponData.AmmoClasses = WeaponDefinition->AmmoClasses;
		WeaponData.MagSize = WeaponDefinition->MagSize;
		WeaponData.Rounds.Empty();
		WeaponData.FireRate = WeaponDefinition->FireRate;
		WeaponData.ReloadTime = WeaponDefinition->ReloadTime;
		WeaponData.bAutomatic = WeaponDefinition->bAutomatic;
		
		SetupRounds(Definition, PredictedData);
	}
}

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	Super::SetupProperties(NewItemId, Definition, Instance);
	
	if (auto WeaponInstance = Cast<UWeaponInstance>(Instance))
	{
		WeaponData.AmmoClasses = WeaponInstance->WeaponData.AmmoClasses;
		WeaponData.MagSize = WeaponInstance->WeaponData.MagSize;
		WeaponData.Rounds = WeaponInstance->WeaponData.Rounds;
		WeaponData.FireRate = WeaponInstance->WeaponData.FireRate;
		WeaponData.ReloadTime = WeaponInstance->WeaponData.ReloadTime;
		WeaponData.bAutomatic = WeaponInstance->WeaponData.bAutomatic;
	}
}

void UWeaponInstance::SetupRounds(const UItemDefinition* Definition, const UItemPredictedData* PredictedData)
{
	if (auto WeaponPredictedData = Cast<UWeaponPredictedData>(PredictedData))
	{
		for (FWeaponPredictedAmmo Ammo : WeaponPredictedData->Ammo)
		{
			if (Ammo.Count <= 0)
			{
				continue;
			}
					
			uint16 LoadedAmmoCount = 0;

			for (UAmmoObject* AmmoObj : WeaponData.Rounds)
			{
				LoadedAmmoCount += AmmoObj->GetAmount();
			}

			if (LoadedAmmoCount < WeaponData.MagSize)
			{
				bool bWasStack = false;
				uint16 AmmoRemain = WeaponData.MagSize - LoadedAmmoCount;

				for (UAmmoObject* AmmoObj : WeaponData.Rounds)
				{
					if (Ammo.Definition == AmmoObj->ItemDefinition)
					{
						AmmoObj->AddAmount(FMath::Clamp(Ammo.Count, 0, AmmoRemain));
						bWasStack = true;
						break;
					}
				}
					
				if (!bWasStack)
				{
					if (UAmmoObject* NewAmmoObject = Cast<UAmmoObject>(
						UItemSystemCore::GenerateItemObject(GetWorld(), Ammo.Definition, nullptr)))
					{
						NewAmmoObject->SetAmount(FMath::Clamp(Ammo.Count, 0, AmmoRemain));
						WeaponData.Rounds.Add(NewAmmoObject);
					}
				}
			}
		}

		if (!WeaponData.Rounds.IsEmpty())
		{
			WeaponData.CurrentAmmoClass = Cast<UAmmoDefinition>(WeaponData.Rounds[0]->ItemDefinition);
		}
	}
	else
	{
		if (WeaponData.AmmoClasses.IsValidIndex(0))
		{
			if (UAmmoObject* AmmoObject = Cast<UAmmoObject>(
				UItemSystemCore::GenerateItemObject(GetWorld(), WeaponData.AmmoClasses[0], nullptr)))
			{
				AmmoObject->SetAmount(WeaponData.MagSize);
				WeaponData.Rounds.Add(AmmoObject);
			}

			WeaponData.CurrentAmmoClass = WeaponData.AmmoClasses[0];
		}
	}
}

void UWeaponObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UWeaponObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
}

void UWeaponObject::OnBindItemActor()
{
	Super::OnBindItemActor();

	if (auto BindWeapon = GetBoundActor<AWeaponActor>())
	{
		BindWeapon->OnWeaponStartAttack.BindUObject(this, &UWeaponObject::OnAttack);
		BindWeapon->OnWeaponStopAttack.BindUObject(this, &UWeaponObject::OnStopAttack);
	}
}

void UWeaponObject::OnUnbindItemActor(AItemActor* PrevItemActor)
{
	Super::OnUnbindItemActor(PrevItemActor);

	if (auto PrevBoundWeapon = Cast<AWeaponActor>(PrevItemActor))
	{
		PrevBoundWeapon->OnWeaponStartAttack.Unbind();
		PrevBoundWeapon->OnWeaponStopAttack.Unbind();
	}
}

bool UWeaponObject::IsSimilar(const UItemObject* OtherItemObject) const
{
	bool bResult = Super::IsSimilar(OtherItemObject);
	if (bResult)
	{
		auto OtherWeaponObject = Cast<UWeaponObject>(OtherItemObject);
		bResult &= OtherWeaponObject != nullptr;
		
		if (OtherWeaponObject)
		{
			UWeaponInstance* OtherWeaponInstance = OtherWeaponObject->GetWeaponInstance();
			bResult &= OtherWeaponInstance != nullptr;
			
			if (OtherWeaponInstance)
			{
				if (UWeaponInstance* MyWeaponInstance = GetWeaponInstance())
				{
					bResult &= MyWeaponInstance->WeaponData.Rounds.Num() == OtherWeaponInstance->WeaponData.Rounds.Num();
					
					uint16 MyAmmoCount = 0;
					uint16 OtherAmmoCount = 0;
					
					for (UAmmoObject* AmmoObj : MyWeaponInstance->WeaponData.Rounds)
					{
						MyAmmoCount += AmmoObj->GetAmount();
					}

					for (UAmmoObject* AmmoObj : OtherWeaponInstance->WeaponData.Rounds)
					{
						OtherAmmoCount += AmmoObj->GetAmount();
					}

					bResult &= MyAmmoCount == OtherAmmoCount;

					if (bResult)
					{
						for (uint8 i = 0; i < MyWeaponInstance->WeaponData.Rounds.Num(); i++)
						{
							bResult &= OtherWeaponInstance->WeaponData.Rounds.IsValidIndex(i);
						
							if (bResult)
							{
								bResult &= MyWeaponInstance->WeaponData.Rounds[i]->IsSimilar(OtherWeaponInstance->
									WeaponData.Rounds[i]);
								bResult &= MyWeaponInstance->WeaponData.Rounds[i]->GetAmount() == OtherWeaponInstance->
									WeaponData.Rounds[i]->GetAmount();
							}

							if (!bResult)
							{
								break;
							}
						}
					}
				}
			}
		}
	}
	return bResult;
}

void UWeaponObject::StartAttack()
{
	if (!CanAttack() || bHoldTrigger)
	{
		return;
	}

	if (bInFireRate)
	{
		CallAttack();
		IsAutomatic() ? SetRepetitiveFireTimer() : SetSingleFireTimer();
	}

	bHoldTrigger = true;
}

void UWeaponObject::CallAttack()
{
	if (!CanAttack())
	{
		StopAttack();
	}
	else if (bInFireRate)
	{
		DecreaseAmmo();
		OnAttack();
		OnAttackStart.Broadcast();
		bInFireRate = false;
	}
}

void UWeaponObject::StopAttack()
{
	if (!bHoldTrigger)
	{
		return;
	}

	OnAttackStop.Broadcast();

	if (IsAutomatic())
	{
		GetWorldTimerManager().ClearTimer(RepeatAttackTimer);
		SetSingleFireTimer();
	}

	bHoldTrigger = false;
	OnStopAttack();
}

void UWeaponObject::StartAlternative()
{
	
}

void UWeaponObject::StopAlternative()
{
	
}

void UWeaponObject::IncreaseAmmo(UAmmoObject* AmmoObject, int Amount)
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;

	if (AmmoData.IsValidIndex(0))
	{
		if (AmmoData[0]->IsSimilar(AmmoObject))
		{
			AmmoData[0]->AddAmount(Amount);
			return;
		}
	}
	
	if (UAmmoObject* NewAmmoObject = Cast<UAmmoObject>(UItemSystemCore::GenerateItemObject(GetWorld(), AmmoObject)))
	{
		NewAmmoObject->SetAmount(Amount);
		AmmoData.Insert(NewAmmoObject, 0);
	}
	
	if (AmmoData.IsValidIndex(0))
	{
		GetWeaponInstance()->WeaponData.CurrentAmmoClass = Cast<UAmmoDefinition>(AmmoData[0]->ItemDefinition);
	}
}

void UWeaponObject::DecreaseAmmo()
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;
	
	if (!AmmoData.IsEmpty() && AmmoData.IsValidIndex(0))
	{
		AmmoData[0]->RemoveAmount(1);
		
		if (AmmoData[0]->GetAmount() == 0)
		{
			AmmoData.RemoveAt(0);
		}
	}
	
	if (AmmoData.IsValidIndex(0))
	{
		GetWeaponInstance()->WeaponData.CurrentAmmoClass = Cast<UAmmoDefinition>(AmmoData[0]->ItemDefinition);
	}
}

int UWeaponObject::CalculateRequiredAmmoCount() const
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;
	uint16 AmmoCount = 0;

	for (UAmmoObject* Ammo : AmmoData)
	{
		AmmoCount += Ammo->GetAmount();
	}
	
	return GetMagSize() - AmmoCount;
}

float UWeaponObject::CalculateFireRate() const
{
	return GetDefaultFireRate();
}

bool UWeaponObject::IsMagFull() const
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;
	uint16 AmmoCount = 0;

	for (UAmmoObject* Ammo : AmmoData)
	{
		AmmoCount += Ammo->GetAmount();
	}
	
	return AmmoCount >= GetMagSize();
}

bool UWeaponObject::IsMagEmpty() const
{
	return GetWeaponInstance()->WeaponData.Rounds.IsEmpty();
}

bool UWeaponObject::CanAttack() const
{
	return HasBoundActor() && !IsMagEmpty();
}

TArray<const UAmmoDefinition*> UWeaponObject::GetAmmoClasses() const
{
	return GetWeaponInstance()->WeaponData.AmmoClasses;
}

int UWeaponObject::GetMagSize() const
{
	return GetWeaponInstance()->WeaponData.MagSize;
}

TArray<UAmmoObject*> UWeaponObject::GetRounds() const
{
	return GetWeaponInstance()->WeaponData.Rounds;
}

const UAmmoDefinition* UWeaponObject::GetCurrentAmmoClass() const
{
	return GetWeaponInstance()->WeaponData.CurrentAmmoClass.Get();
}

float UWeaponObject::GetReloadTime() const
{
	return GetWeaponInstance()->WeaponData.ReloadTime;
}

float UWeaponObject::GetDefaultFireRate() const
{
	int FireRate = GetWeaponInstance()->WeaponData.FireRate;
	return 1.0f / (FireRate / 60.0f);
}

bool UWeaponObject::IsAutomatic() const
{
	return GetWeaponInstance()->WeaponData.bAutomatic;
}

AWeaponActor* UWeaponObject::GetWeaponActor() const
{
	return GetBoundActor<AWeaponActor>();
}

UWeaponInstance* UWeaponObject::GetWeaponInstance() const
{
	return GetItemInstance<UWeaponInstance>();
}

void UWeaponObject::SetSingleFireTimer()
{
	FTimerDelegate CanAttackDelegate;
	CanAttackDelegate.BindLambda([&, this]
	{
		bInFireRate = true;
		GetWorldTimerManager().ClearTimer(CanAttackTimer);
	});
		
	GetWorldTimerManager().SetTimer(CanAttackTimer, CanAttackDelegate, CalculateFireRate(), false);
}

void UWeaponObject::SetRepetitiveFireTimer()
{
	FTimerDelegate RepeatAttackDelegate;
	RepeatAttackDelegate.BindLambda([&, this]
	{
		bInFireRate = true;
		CallAttack();
	});
	
	GetWorldTimerManager().SetTimer(RepeatAttackTimer, RepeatAttackDelegate, CalculateFireRate(), true);
}

void UWeaponObject::OnAttack_Implementation()
{

}

void UWeaponObject::OnStopAttack_Implementation()
{
	
}
