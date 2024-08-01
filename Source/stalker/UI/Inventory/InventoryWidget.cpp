// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "UI/Inventory/ItemsContainerGridWidget.h"

void UInventoryWidget::InitializeInventory(UItemsContainerComponent* ItemsContainerComponent)
{
	check(ItemsContainerComponent);
	
	ItemsGrid->SetupContainerGrid(ItemsContainerComponent);
}
