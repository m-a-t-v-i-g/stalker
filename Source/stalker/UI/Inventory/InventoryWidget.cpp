// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "InventoryGridWidget.h"

void UInventoryWidget::SetupInventory(UInventoryManagerComponent* PlayerInventoryManager, UItemsContainer* ItemsContainer)
{
	check(PlayerInventoryManager);
	check(ItemsContainer);
	
	ItemsGrid->SetupContainerGrid(PlayerInventoryManager, ItemsContainer);
}

void UInventoryWidget::ClearInventory()
{
	ItemsGrid->ClearContainerGrid();
}
