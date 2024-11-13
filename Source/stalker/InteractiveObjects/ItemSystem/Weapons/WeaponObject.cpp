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

void UWeaponObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UWeaponObject, WeaponParams, COND_OwnerOnly);
}

void UWeaponObject::OnBindItemActor()
{
	Super::OnBindItemActor();

	if (auto BindWeapon = GetBoundItem<AWeaponActor>())
	{
		BindWeapon->OnWeaponStartAttack.BindUObject(this, &UWeaponObject::OnWeaponAttackStart);
		BindWeapon->OnWeaponStopAttack.BindUObject(this, &UWeaponObject::OnWeaponAttackStop);
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

void UWeaponObject::StartAttack()
{
	DecreaseAmmo();

	OnWeaponAttackStart();
}

void UWeaponObject::StopAttack()
{
	OnWeaponAttackStop();
}

void UWeaponObject::OnWeaponAttackStart_Implementation()
{

}

void UWeaponObject::OnWeaponAttackStop_Implementation()
{
	
}

void UWeaponObject::ReloadAmmo()
{
	WeaponParams.Rounds = GetMagSize();
}

void UWeaponObject::IncreaseAmmo(int Amount)
{
	WeaponParams.Rounds = FMath::Clamp(WeaponParams.Rounds + Amount, 0, GetMagSize());
}

void UWeaponObject::DecreaseAmmo()
{
	WeaponParams.Rounds = FMath::Clamp(WeaponParams.Rounds - 1, 0, GetMagSize());
}

int UWeaponObject::CalculateRequiredAmmoCount() const
{
	return FMath::Clamp(GetMagSize() - WeaponParams.Rounds, 0, GetMagSize());
}

bool UWeaponObject::IsAutomatic() const
{
	return WeaponParams.bAutomatic;
}

bool UWeaponObject::IsMagFull() const
{
	return WeaponParams.Rounds >= GetMagSize();
}

bool UWeaponObject::IsMagEmpty() const
{
	return WeaponParams.Rounds <= 0;
}

bool UWeaponObject::CanAttack() const
{
	return !IsMagEmpty();
}

int UWeaponObject::GetMagSize() const
{
	return 0; // TODO;
}

const FWeaponParams& UWeaponObject::GetWeaponParams() const
{
	return WeaponParams;
}
