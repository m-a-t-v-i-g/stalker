// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "InventoryGridWidget.h"

void UInventoryWidget::SetupInventory(UInventoryComponent* InventoryComp, UInventoryManagerComponent* InventoryManager)
{
	check(InventoryManager);
	check(InventoryComp);
	
	ItemsGrid->SetupContainerGrid(InventoryComp, InventoryManager);
}

void UInventoryWidget::ClearInventory()
{
	ItemsGrid->ClearContainerGrid();
}
