// Fill out your copyright notice in the Description page of Project Settings.

#include "ArmorObject.h"
#include "Components/Inventory/CharacterInventoryComponent.h"

void UArmorObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
}

USkeletalMesh* UArmorObject::GetDefaultMesh() const
{
	USkeletalMesh* SkeletalMesh = nullptr;
	if (GetRow<FTableRowArmor>())
	{
		SkeletalMesh = GetRow<FTableRowArmor>()->DefaultMesh;
	}
	return SkeletalMesh;
}

USkeletalMesh* UArmorObject::GetHelmetMesh() const
{
	USkeletalMesh* SkeletalMesh = nullptr;
	if (GetRow<FTableRowArmor>())
	{
		SkeletalMesh = GetRow<FTableRowArmor>()->HelmetMesh;
	}
	return SkeletalMesh;
}
