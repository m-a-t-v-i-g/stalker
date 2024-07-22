// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "UI/Items/ItemsContainerGridWidget.h"

void UInventoryWidget::InitializeInventory(UItemsContainerComponent* ItemsContainerComponent)
{
	ItemsGrid->SetupContainerGrid(ItemsContainerComponent);
}
