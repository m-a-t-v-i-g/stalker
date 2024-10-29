// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "InteractiveObjects/ItemSystem/ItemObject.h"
#include "Interfaces/UsableInterface.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	IUsableInterface::Execute_Use(ItemObject, this);
}
