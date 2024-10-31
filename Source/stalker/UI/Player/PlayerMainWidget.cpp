// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMainWidget.h"
#include "PlayerManagerWidget.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"

void UPlayerMainWidget::OpenInventory()
{
	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UPlayerManagerWidget>(GetOwningPlayer(), APlayerHUD::StaticInventoryWidgetClass);
		
		if (InventoryWidget)
		{
			InventoryWidget->InitializeManager(OwnAbilityComponent.Get(), OwnInventoryComponent.Get());
			SlotManager->AddChild(InventoryWidget);
		}
		
		ToggleTab(EHUDTab::Inventory);
	}
}

void UPlayerMainWidget::CloseInventory()
{
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
	}

	ToggleTab(EHUDTab::HUD);
}

void UPlayerMainWidget::StartLooting(UItemsContainerComponent* LootItemsContainer)
{
	
}

void UPlayerMainWidget::InitializeMainWidget(UOrganicAbilityComponent* AbilityComp,
                                             UCharacterInventoryComponent* InventoryComp)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = InventoryComp;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());

	HUD->InitializeHUD(OwnAbilityComponent.Get());
}

void UPlayerMainWidget::ToggleTab(EHUDTab ActivateTab)
{
	switch (ActivateTab)
	{
	case EHUDTab::Inventory:
		TabSwitcher->SetActiveWidget(SlotManager);
		break;
	default:
		TabSwitcher->SetActiveWidget(HUD);
		break;
	}
}
