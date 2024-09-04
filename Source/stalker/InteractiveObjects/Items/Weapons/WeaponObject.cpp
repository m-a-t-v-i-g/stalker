// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "WeaponActor.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Net/UnrealNetwork.h"

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

void UWeaponObject::InitItem(const uint32 ItemId, const FDataTableRowHandle& RowHandle)
{
	Super::InitItem(ItemId, RowHandle);
	if (auto Row = GetRow<FTableRowWeapon>())
	{
		WeaponParams.AmmoClasses = Row->AmmoClasses;
		WeaponParams.Rounds = Row->MagSize;
		WeaponParams.FireRate = Row->FireRate;
		WeaponParams.ReloadTime = Row->ReloadTime;
		WeaponParams.bAutomatic = Row->bAutomatic;
	}
}

void UWeaponObject::InitItem(const uint32 ItemId, const UItemObject* ItemObject)
{
	Super::InitItem(ItemId, ItemObject);
	if (auto WeaponObject = Cast<UWeaponObject>(ItemObject))
	{
		WeaponParams = WeaponObject->GetWeaponParams();
	}
}

void UWeaponObject::OnBindItem()
{
	Super::OnBindItem();

	if (auto BindWeapon = GetBoundItem<AWeaponActor>())
	{
		BindWeapon->OnWeaponStartAttack.BindUObject(this, &UWeaponObject::OnWeaponAttackStart);
		BindWeapon->OnWeaponStopAttack.BindUObject(this, &UWeaponObject::OnWeaponAttackStop);
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
			bResult &= WeaponParams == OtherWeaponObject->GetWeaponParams();
		}
	}
	return bResult;
}

void UWeaponObject::OnWeaponAttackStart_Implementation()
{
	DecreaseAmmo();
}

void UWeaponObject::OnWeaponAttackStop_Implementation()
{
	
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
	int MagSize = 0;
	if (GetRow<FTableRowWeapon>())
	{
		MagSize = GetRow<FTableRowWeapon>()->MagSize;
	}
	return MagSize;
}

const FWeaponParams& UWeaponObject::GetWeaponParams() const
{
	return WeaponParams;
}
