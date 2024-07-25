// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerManagerWidget.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/Inventory/CharacterInventoryComponent.h"
#include "UI/Inventory/CharacterEquipmentWidget.h"
#include "UI/Inventory/InventoryWidget.h"

void UPlayerManagerWidget::InitializeManager(UStalkerAbilityComponent* AbilityComp,
                                             UCharacterInventoryComponent* CharInventoryComp)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = CharInventoryComp;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());
	
	Inventory->InitializeInventory(CharInventoryComp);
	Equipment->InitializeCharacterEquipment(CharInventoryComp);
}
