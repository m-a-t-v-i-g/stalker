// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMainWidget.h"
#include "PlayerManagerWidget.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Inventory/CharacterInventoryComponent.h"
#include "HUD/HUDWidget.h"

void UPlayerMainWidget::InitializeMainWidget(UStalkerAbilityComponent* AbilityComp, UCharacterInventoryComponent* InventoryComp)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = InventoryComp;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());

	HUD->InitializeHUD(OwnAbilityComponent.Get());
	Manager->InitializeManager(OwnAbilityComponent.Get(), OwnInventoryComponent.Get());
}

void UPlayerMainWidget::ToggleTab(EActivateTab ActivateTab)
{
	switch (ActivateTab)
	{
	case EActivateTab::Inventory:
		TabSwitcher->SetActiveWidget(Manager);
		break;
	case EActivateTab::PDA:
		TabSwitcher->SetActiveWidget(PDA);
		break;
	default:
		TabSwitcher->SetActiveWidget(HUD);
		break;
	}
}

void UPlayerMainWidget::StartLooting(UInventoryComponent* LootInventory)
{
	Manager->StartLooting(LootInventory);
}
