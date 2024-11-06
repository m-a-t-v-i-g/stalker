// Fill out your copyright notice in the Description page of Project Settings.

#include "ArmorObject.h"
#include "Character/CharacterInventoryComponent.h"

void UArmorObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
}

USkeletalMesh* UArmorObject::GetVisual() const
{
	return nullptr; // TODO;
}
