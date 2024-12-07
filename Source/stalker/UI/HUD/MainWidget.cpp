// Fill out your copyright notice in the Description page of Project Settings.

#include "MainWidget.h"
#include "Inventory/InventoryManagerWidget.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"
#include "Player/InventoryManagerComponent.h"

void UMainWidget::InitializeMainWidget(const FPlayerInitInfo& CharacterInitInfo)
{
	OwnAbilityComponent = CharacterInitInfo.AbilitySystemComponent;
	OwnInventoryManager = CharacterInitInfo.InventoryManager;
	OwnInventoryComponent = CharacterInitInfo.InventoryComponent;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());

	HUD->InitializeHUD(CharacterInitInfo);
}

void UMainWidget::SetupOwnInventory()
{
	if (!InventoryManagerWidget)
	{
		InventoryManagerWidget = CreateWidget<UInventoryManagerWidget>(GetOwningPlayer(), AStalkerHUD::StaticInventoryWidgetClass);
		
		if (InventoryManagerWidget)
		{
			InventoryManagerWidget->OpenInventory(OwnAbilityComponent.Get(), OwnInventoryComponent.Get(), OwnInventoryManager.Get());
			SlotManager->AddChild(InventoryManagerWidget);
		}
	}
}

void UMainWidget::CloseOwnInventory()
{
	if (InventoryManagerWidget)
	{
		InventoryManagerWidget->RemoveFromParent();
		InventoryManagerWidget->CloseInventory();
		InventoryManagerWidget = nullptr;
	}
}

void UMainWidget::OpenEmptyTab()
{
	if (InventoryManagerWidget)
	{
		InventoryManagerWidget->OpenEmpty();
	}
}

void UMainWidget::OpenLootingTab(UInventoryComponent* InventoryToLoot)
{
	if (InventoryManagerWidget)
	{
		InventoryManagerWidget->OpenLooting(InventoryToLoot);
	}
}

void UMainWidget::OpenHUDTab()
{
	TabSwitcher->SetActiveWidget(HUD);
}

void UMainWidget::OpenInventoryTab()
{
	TabSwitcher->SetActiveWidget(SlotManager);
}
