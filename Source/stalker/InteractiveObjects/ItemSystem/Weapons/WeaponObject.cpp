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

	if (auto BindWeapon = GetBoundItem<AWeaponActor>())
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
			auto OtherWeaponInstance = OtherWeaponObject->GetItemInstance<UWeaponInstance>();
			bResult &= OtherWeaponInstance != nullptr;
			
			if (OtherWeaponInstance)
			{
				if (auto MyWeaponInstance = GetItemInstance<UWeaponInstance>())
				{
					// TODO:
					//bResult &= MyWeaponInstance->Rounds == OtherWeaponInstance->Rounds;
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
	
	if (UAmmoObject* NewAmmoObject = Cast<UAmmoObject>(UItemSystemCore::GenerateItemObject(GetWorld(), AmmoObject)))
	{
		NewAmmoObject->SetAmount(Amount);
		AmmoData.Add(NewAmmoObject);
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
