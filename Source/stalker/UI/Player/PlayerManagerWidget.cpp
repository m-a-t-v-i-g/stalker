// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerManagerWidget.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/Inventory/InventoryComponent.h"
#include "UI/Inventory/InventoryWidget.h"

void UPlayerManagerWidget::InitializeManager(UStalkerAbilityComponent* AbilityComp, UInventoryComponent* InventoryComp)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = InventoryComp;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());
	
	Inventory->InitializeInventory(InventoryComp);
}
