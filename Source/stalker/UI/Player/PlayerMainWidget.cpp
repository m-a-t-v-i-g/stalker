// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMainWidget.h"
#include "PlayerInventoryWidget.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"

void UPlayerMainWidget::OpenInventory()
{
	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UPlayerInventoryWidget>(GetOwningPlayer(), AStalkerHUD::StaticInventoryWidgetClass);
		
		if (InventoryWidget)
		{
			InventoryWidget->OpenInventory(OwnAbilityComponent.Get(), OwnInventoryComponent.Get());
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
		InventoryWidget->CloseInventory();
		InventoryWidget = nullptr;
	}

	ToggleTab(EHUDTab::HUD);
}

void UPlayerMainWidget::StartLooting(UInventoryComponent* InventoryToLoot)
{
	if (InventoryWidget)
	{
		InventoryWidget->OpenLooting(InventoryToLoot);
	}
}

void UPlayerMainWidget::InitializeMainWidget(const FCharacterInitInfo& CharacterInitInfo)
{
	OwnAbilityComponent = CharacterInitInfo.AbilitySystemComponent;
	OwnInventoryComponent = CharacterInitInfo.InventoryComponent;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());

	HUD->InitializeHUD(CharacterInitInfo);
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
