// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInventoryWidget.h"
#include "UI/Items/ItemsContainerGridWidget.h"

void UPlayerInventoryWidget::InitializeInventory(UItemsContainerComponent* ItemsContainerComponent)
{
	InventoryGrid->SetupContainerGrid(ItemsContainerComponent);
}
