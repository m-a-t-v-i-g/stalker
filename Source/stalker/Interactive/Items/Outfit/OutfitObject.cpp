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
