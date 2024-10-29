// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "ContainerGridWidget.h"

void UInventoryWidget::InitializeInventory(UItemsContainer* ItemsContainerComponent)
{
	check(ItemsContainerComponent);
	
	ItemsGrid->SetupContainerGrid(ItemsContainerComponent);
}
