// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Interfaces/UsableInterface.h"

UInventoryComponent::UInventoryComponent()
{
}

void UInventoryComponent::DropItem(UItemObject* ItemObject)
{
}

void UInventoryComponent::UseItem(UItemObject* ItemObject)
{
	Server_UseItem(ItemObject);
}

void UInventoryComponent::Server_UseItem_Implementation(UItemObject* ItemObject)
{
	if (IUsableInterface* UsableItem = Cast<IUsableInterface>(ItemObject))
	{
		UsableItem->Use(this);
	}
}
