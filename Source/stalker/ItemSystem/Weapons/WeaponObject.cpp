// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "ItemSystemCore.h"
#include "WeaponActor.h"
#include "Ammo/AmmoObject.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UWeaponInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, WeaponData, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, DamageData, COND_OwnerOnly);
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
	if (auto WeaponDefinition = Cast<UWeaponDefinition>(Definition))
	{
		WeaponData.AmmoClasses = WeaponDefinition->AmmoClasses;
		WeaponData.MagSize = WeaponDefinition->MagSize;
		WeaponData.Rounds.Empty();
		WeaponData.FireRate = WeaponDefinition->FireRate;
		WeaponData.ReloadTime = WeaponDefinition->ReloadTime;
		WeaponData.bAutomatic = WeaponDefinition->bAutomatic;
		
		DamageData = WeaponDefinition->DamageData;
		
		SetupRounds(Definition, PredictedData);
	}
	
	Super::SetupProperties(NewItemId, Definition, PredictedData);
}

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	if (auto WeaponInstance = Cast<UWeaponInstance>(Instance))
	{
		WeaponData.AmmoClasses = WeaponInstance->WeaponData.AmmoClasses;
		WeaponData.MagSize = WeaponInstance->WeaponData.MagSize;
		WeaponData.Rounds.Empty();
		WeaponData.FireRate = WeaponInstance->WeaponData.FireRate;
		WeaponData.ReloadTime = WeaponInstance->WeaponData.ReloadTime;
		WeaponData.bAutomatic = WeaponInstance->WeaponData.bAutomatic;

		DamageData = WeaponInstance->DamageData;

		SetupRounds(Instance);
	}
	
	Super::SetupProperties(NewItemId, Definition, Instance);
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
		}
	}
	
	DefineCurrentAmmoClass();
}

void UWeaponInstance::SetupRounds(const UItemInstance* Instance)
{
	if (auto WeaponInstance = Cast<UWeaponInstance>(Instance))
	{
		for (UAmmoObject* Ammo : WeaponInstance->WeaponData.Rounds)
		{ 
			if (UAmmoObject* NewAmmoObject = Cast<UAmmoObject>(UItemSystemCore::GenerateItemObject(GetWorld(), Ammo)))
			{
				WeaponData.Rounds.Add(NewAmmoObject);
			}
		}
	}

	DefineCurrentAmmoClass();
}

void UWeaponInstance::DefineCurrentAmmoClass()
{
	if (!WeaponData.Rounds.IsEmpty())
	{
		WeaponData.CurrentAmmoClass = Cast<UAmmoDefinition>(WeaponData.Rounds[0]->ItemDefinition);
	}
	else
	{
		WeaponData.CurrentAmmoClass = WeaponData.AmmoClasses[0];
	}
}

void UWeaponObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UWeaponObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	// TODO: try use item
}

void UWeaponObject::OnBindItemActor()
{
	Super::OnBindItemActor();

	if (auto BindWeapon = GetBoundActor<AWeaponActor>())
	{
		
	}
}

void UWeaponObject::OnUnbindItemActor(AItemActor* PrevItemActor)
{
	Super::OnUnbindItemActor(PrevItemActor);

	if (auto PrevBoundWeapon = Cast<AWeaponActor>(PrevItemActor))
	{
		
	}
}

bool UWeaponObject::IsCorrespondsTo(const UItemObject* OtherItemObject) const
{
	bool bResult = Super::IsCorrespondsTo(OtherItemObject);
	if (bResult)
	{
		auto OtherWeaponObject = Cast<UWeaponObject>(OtherItemObject);
		bResult &= OtherWeaponObject != nullptr;
		
		if (bResult && OtherWeaponObject)
		{
			UWeaponInstance* OtherWeaponInstance = OtherWeaponObject->GetWeaponInstance();
			bResult &= OtherWeaponInstance != nullptr;
			
			if (bResult && OtherWeaponInstance)
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
								bResult &= MyWeaponInstance->WeaponData.Rounds[i]->IsCorrespondsTo(OtherWeaponInstance->
									WeaponData.Rounds[i]);
								bResult &= MyWeaponInstance->WeaponData.Rounds[i]->GetAmount() == OtherWeaponInstance->
									WeaponData.Rounds[i]->GetAmount();
							}

							if (!bResult)
							{
								break;
							}
						}

						bResult &= MyWeaponInstance->DamageData == OtherWeaponInstance->DamageData;
					}
				}
			}
		}
	}
	return bResult;
}

void UWeaponObject::StartFire()
{
	DecreaseAmmo();
	OnAttackStart.Broadcast();
	OnFireStart();
}

void UWeaponObject::StopFire()
{
	OnAttackStop.Broadcast();
	OnFireStop();
}

void UWeaponObject::IncreaseAmmo(UAmmoObject* AmmoObject, int Amount)
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;

	if (AmmoData.IsValidIndex(0))
	{
		if (AmmoData[0]->IsCorrespondsTo(AmmoObject))
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

void UWeaponObject::CancelAllActions()
{
	CancelAllActionsDelegate.Broadcast();
}

int UWeaponObject::CalculateRequiredAmmoCount() const
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;
	uint16 AmmoCount = 0;

	for (UAmmoObject* Ammo : AmmoData)
	{
		AmmoCount += Ammo->GetAmount();
	}
	
	return GetDefaultMagSize() - AmmoCount;
}

bool UWeaponObject::IsMagFull() const
{
	TArray<UAmmoObject*>& AmmoData = GetWeaponInstance()->WeaponData.Rounds;
	uint16 AmmoCount = 0;

	for (UAmmoObject* Ammo : AmmoData)
	{
		AmmoCount += Ammo->GetAmount();
	}
	
	return AmmoCount >= GetDefaultMagSize();
}

bool UWeaponObject::IsMagEmpty() const
{
	return GetWeaponInstance()->WeaponData.Rounds.IsEmpty();
}

bool UWeaponObject::CanAttack() const
{
	return HasBoundActor() && !IsMagEmpty();
}

const UWeaponDefinition* UWeaponObject::GetWeaponDefinition() const
{
	return Cast<UWeaponDefinition>(GetDefinition());
}

TArray<const UAmmoDefinition*> UWeaponObject::GetAmmoClasses() const
{
	return GetWeaponInstance()->WeaponData.AmmoClasses;
}

int UWeaponObject::GetDefaultMagSize() const
{
	return GetWeaponDefinition()->MagSize;
}

TArray<UAmmoObject*> UWeaponObject::GetRounds() const
{
	return GetWeaponInstance()->WeaponData.Rounds;
}

uint16 UWeaponObject::GetRemainedRounds() const
{
	uint16 RoundsRemain = 0;
	for (UAmmoObject* Ammo : GetRounds())
	{
		RoundsRemain += Ammo->GetAmount();
	}
	return RoundsRemain;
}

const UAmmoDefinition* UWeaponObject::GetCurrentAmmoClass() const
{
	return GetWeaponInstance()->WeaponData.CurrentAmmoClass.Get();
}

float UWeaponObject::GetReloadTime() const
{
	return CalculateReloadTime();
}

float UWeaponObject::GetDefaultReloadTime() const
{
	return GetWeaponDefinition()->ReloadTime;
}

float UWeaponObject::GetFireRate() const
{
	return CalculateFireRate();
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

FWeaponDamageData UWeaponObject::GetDamageData() const
{
	return GetWeaponInstance()->DamageData;
}

float UWeaponObject::GetSpreadExponent() const
{
	return CalculateSpreadExponent();
}

float UWeaponObject::GetDefaultSpreadExponent() const
{
	return GetWeaponDefinition()->SpreadExponent;
}

AWeaponActor* UWeaponObject::GetWeaponActor() const
{
	return GetBoundActor<AWeaponActor>();
}

UWeaponInstance* UWeaponObject::GetWeaponInstance() const
{
	return GetItemInstance<UWeaponInstance>();
}

void UWeaponObject::OnFireStart_Implementation()
{

}

void UWeaponObject::OnFireStop_Implementation()
{
	
}

float UWeaponObject::CalculateReloadTime() const
{
	return GetDefaultReloadTime();
}

float UWeaponObject::CalculateFireRate() const
{
	return GetDefaultFireRate();
}

float UWeaponObject::CalculateSpreadExponent() const
{
	return GetDefaultSpreadExponent();
}
