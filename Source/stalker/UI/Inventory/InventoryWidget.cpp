// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "InventoryGridWidget.h"

void UInventoryWidget::SetupInventory(UInventoryComponent* InventoryComp)
{
	check(InventoryComp);
	
	ItemsGrid->SetupContainerGrid(InventoryComp);
}

void UInventoryWidget::ClearInventory()
{
	ItemsGrid->ClearContainerGrid();
}
