// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "WeaponActor.h"
#include "Character/CharacterInventoryComponent.h"
#include "Net/UnrealNetwork.h"

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                      const UItemPredictedData* PredictedData)
{
	if (auto WeaponDefinition = Cast<UWeaponDefinition>(Definition))
	{
		Super::SetupProperties(NewItemId, Definition, PredictedData);

		AmmoClasses = WeaponDefinition->AmmoClasses;
		MagSize = WeaponDefinition->MagSize;
		Rounds = WeaponDefinition->MagSize;
		FireRate = WeaponDefinition->FireRate;
		ReloadTime = WeaponDefinition->ReloadTime;
		bAutomatic = WeaponDefinition->bAutomatic;

		if (auto WeaponPredictedData = Cast<UWeaponPredictedData>(PredictedData))
		{
			Rounds = FMath::Clamp(Rounds, 0, WeaponPredictedData->Rounds);
		}
	}
}

void UWeaponInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	if (auto WeaponInstance = Cast<UWeaponInstance>(Instance))
	{
		Super::SetupProperties(NewItemId, Definition, Instance);

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
					bResult &= MyWeaponInstance->Rounds == OtherWeaponInstance->Rounds;
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

void UWeaponObject::ReloadAmmo()
{
	GetItemInstance<UWeaponInstance>()->Rounds = GetMagSize();
}

void UWeaponObject::IncreaseAmmo(int Amount)
{
	int OldRounds = GetItemInstance<UWeaponInstance>()->Rounds;
	GetItemInstance<UWeaponInstance>()->Rounds = FMath::Clamp(OldRounds + Amount, 0, GetMagSize());
}

void UWeaponObject::DecreaseAmmo()
{
	int OldRounds = GetItemInstance<UWeaponInstance>()->Rounds;
	GetItemInstance<UWeaponInstance>()->Rounds = FMath::Clamp(OldRounds - 1, 0, GetMagSize());
}

int UWeaponObject::CalculateRequiredAmmoCount() const
{
	int Rounds = GetItemInstance<UWeaponInstance>()->Rounds;
	return FMath::Clamp(GetMagSize() - Rounds, 0, GetMagSize());
}

float UWeaponObject::CalculateFireRate() const
{
	return GetDefaultFireRate();
}

float UWeaponObject::GetReloadTime() const
{
	return GetItemInstance<UWeaponInstance>()->ReloadTime;
}

float UWeaponObject::GetDefaultFireRate() const
{
	int FireRate = GetItemInstance<UWeaponInstance>()->FireRate;
	return 1.0f / (FireRate / 60.0f);
}

bool UWeaponObject::IsAutomatic() const
{
	return GetItemInstance<UWeaponInstance>()->bAutomatic;
}

bool UWeaponObject::IsMagFull() const
{
	int Rounds = GetItemInstance<UWeaponInstance>()->Rounds;
	return Rounds == GetMagSize();
}

bool UWeaponObject::IsMagEmpty() const
{
	int Rounds = GetItemInstance<UWeaponInstance>()->Rounds;
	return Rounds == 0;
}

bool UWeaponObject::CanAttack() const
{
	return HasBoundActor() && !IsMagEmpty();
}

int UWeaponObject::GetMagSize() const
{
	return GetItemInstance<UWeaponInstance>()->MagSize;
}
