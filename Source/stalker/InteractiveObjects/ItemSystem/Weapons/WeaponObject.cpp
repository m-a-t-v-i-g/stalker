// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "ItemSystemCore.h"
#include "WeaponActor.h"
#include "Ammo/AmmoObject.h"
#include "Character/CharacterInventoryComponent.h"
#include "Net/UnrealNetwork.h"

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                      const UItemPredictedData* PredictedData)
{
	Super::SetupProperties(NewItemId, Definition, PredictedData);

	if (auto WeaponDefinition = Cast<UWeaponDefinition>(Definition))
	{
		AmmoClasses = WeaponDefinition->AmmoClasses;
		MagSize = WeaponDefinition->MagSize;
		FireRate = WeaponDefinition->FireRate;
		ReloadTime = WeaponDefinition->ReloadTime;
		bAutomatic = WeaponDefinition->bAutomatic;

		Rounds.Empty();
		
		if (auto WeaponPredictedData = Cast<UWeaponPredictedData>(PredictedData))
		{
			for (FAmmoStartingData Ammo : WeaponPredictedData->Ammo)
			{
				if (Ammo.Count <= 0)
				{
					continue;
				}
					
				uint16 AmmoCount = 0;

				for (UAmmoObject* AmmoObj : Rounds)
				{
					AmmoCount += AmmoObj->GetAmount();
				}

				if (AmmoCount < MagSize)
				{
					bool bWasStack = false;
					uint16 AmmoRemain = MagSize - AmmoCount;

					for (UAmmoObject* AmmoObj : Rounds)
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
							Rounds.Add(NewAmmoObject);
						}
					}
				}
			}

			if (!Rounds.IsEmpty())
			{
				CurrentAmmoClass = Cast<UAmmoDefinition>(Rounds[0]->ItemDefinition);
			}
		}
		else
		{
			if (AmmoClasses.IsValidIndex(0))
			{
				if (UAmmoObject* AmmoObject = Cast<UAmmoObject>(
					UItemSystemCore::GenerateItemObject(GetWorld(), AmmoClasses[0], nullptr)))
				{
					AmmoObject->SetAmount(MagSize);
					Rounds.Add(AmmoObject);
				}

				CurrentAmmoClass = AmmoClasses[0];
			}
		}
	}
}

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	Super::SetupProperties(NewItemId, Definition, Instance);
	
	if (auto WeaponInstance = Cast<UWeaponInstance>(Instance))
	{
		AmmoClasses = WeaponInstance->AmmoClasses;
		MagSize = WeaponInstance->MagSize;
		Rounds = WeaponInstance->Rounds;
		FireRate = WeaponInstance->FireRate;
		ReloadTime = WeaponInstance->ReloadTime;
		bAutomatic = WeaponInstance->bAutomatic;
	}
}

void UWeaponObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
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

void UWeaponObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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
					bResult &= MyWeaponInstance->Rounds.Num() == OtherWeaponInstance->Rounds.Num();
					
					uint16 MyAmmoCount = 0;
					uint16 OtherAmmoCount = 0;
					
					for (UAmmoObject* AmmoObj : MyWeaponInstance->Rounds)
					{
						MyAmmoCount += AmmoObj->GetAmount();
					}

					for (UAmmoObject* AmmoObj : OtherWeaponInstance->Rounds)
					{
						OtherAmmoCount += AmmoObj->GetAmount();
					}

					bResult &= MyAmmoCount == OtherAmmoCount;

					if (bResult)
					{
						for (uint8 i = 0; i < MyWeaponInstance->Rounds.Num(); i++)
						{
							bResult &= OtherWeaponInstance->Rounds.IsValidIndex(i);
						
							if (bResult)
							{
								bResult &= MyWeaponInstance->Rounds[i]->IsSimilar(OtherWeaponInstance->Rounds[i]);
								bResult &= MyWeaponInstance->Rounds[i]->GetAmount() == OtherWeaponInstance->Rounds[i]->GetAmount();
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

void UWeaponObject::OnAttack_Implementation()
{

}

void UWeaponObject::OnStopAttack_Implementation()
{
	
}

void UWeaponObject::IncreaseAmmo(UAmmoObject* AmmoObject, int Amount)
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->Rounds;

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
		GetWeaponInstance()->CurrentAmmoClass = Cast<UAmmoDefinition>(AmmoData[0]->ItemDefinition);
	}
}

void UWeaponObject::DecreaseAmmo()
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->Rounds;
	
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
		GetWeaponInstance()->CurrentAmmoClass = Cast<UAmmoDefinition>(AmmoData[0]->ItemDefinition);
	}
}

int UWeaponObject::CalculateRequiredAmmoCount() const
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->Rounds;
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

const UAmmoDefinition* UWeaponObject::GetLastAmmoClass() const
{
	return GetWeaponInstance()->CurrentAmmoClass.Get();
}

float UWeaponObject::GetReloadTime() const
{
	return GetWeaponInstance()->ReloadTime;
}

float UWeaponObject::GetDefaultFireRate() const
{
	int FireRate = GetWeaponInstance()->FireRate;
	return 1.0f / (FireRate / 60.0f);
}

bool UWeaponObject::IsAutomatic() const
{
	return GetWeaponInstance()->bAutomatic;
}

bool UWeaponObject::IsMagFull() const
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->Rounds;
	uint16 AmmoCount = 0;

	for (UAmmoObject* Ammo : AmmoData)
	{
		AmmoCount += Ammo->GetAmount();
	}
	
	return AmmoCount >= GetMagSize();
}

bool UWeaponObject::IsMagEmpty() const
{
	return GetWeaponInstance()->Rounds.IsEmpty();
}

bool UWeaponObject::CanAttack() const
{
	return HasBoundActor() && !IsMagEmpty();
}

int UWeaponObject::GetMagSize() const
{
	return GetWeaponInstance()->MagSize;
}
