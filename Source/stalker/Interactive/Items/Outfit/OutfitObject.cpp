// Fill out your copyright notice in the Description page of Project Settings.

#include "OutfitObject.h"
#include "Components/Inventory/CharacterInventoryComponent.h"

void UOutfitObject::Use(UObject* Source)
{
	Super::Use(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
}

USkeletalMesh* UOutfitObject::GetDefaultMesh() const
{
	USkeletalMesh* SkeletalMesh = nullptr;
	if (GetRow<FTableRowArmor>())
	{
		SkeletalMesh = GetRow<FTableRowArmor>()->DefaultMesh;
	}
	return SkeletalMesh;
}

USkeletalMesh* UOutfitObject::GetHelmetMesh() const
{
	USkeletalMesh* SkeletalMesh = nullptr;
	if (GetRow<FTableRowArmor>())
	{
		SkeletalMesh = GetRow<FTableRowArmor>()->HelmetMesh;
	}
	return SkeletalMesh;
}
