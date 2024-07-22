// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerManagerWidget.h"
#include "UI/Inventory/InventoryWidget.h"

void UPlayerManagerWidget::InitializeInventory(UItemsContainerComponent* ItemsContainerComponent)
{
	PlayerInventory->InitializeInventory(ItemsContainerComponent);
}
