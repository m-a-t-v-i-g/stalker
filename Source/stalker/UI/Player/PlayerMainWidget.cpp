// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMainWidget.h"
#include "PlayerInventoryWidget.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/NamedSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"
#include "Player/PlayerInventoryManagerComponent.h"

void UPlayerMainWidget::InitializeMainWidget(const FPlayerCharacterInitInfo& CharacterInitInfo)
{
	OwnAbilityComponent = CharacterInitInfo.AbilitySystemComponent;
	OwnInventoryManager = CharacterInitInfo.InventoryManager;
	OwnInventoryComponent = CharacterInitInfo.InventoryComponent;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());

	HUD->InitializeHUD(CharacterInitInfo);
}

void UPlayerMainWidget::SetupOwnInventory()
{
	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UPlayerInventoryWidget>(GetOwningPlayer(), AStalkerHUD::StaticInventoryWidgetClass);
		
		if (InventoryWidget)
		{
			InventoryWidget->OpenInventory(OwnAbilityComponent.Get(), OwnInventoryComponent.Get(), OwnInventoryManager.Get());
			SlotManager->AddChild(InventoryWidget);
		}
	}
}

void UPlayerMainWidget::CloseOwnInventory()
{
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget->CloseInventory();
		InventoryWidget = nullptr;
	}
}

void UPlayerMainWidget::OpenEmptyTab()
{
	if (InventoryWidget)
	{
		InventoryWidget->OpenEmpty();
	}
}

void UPlayerMainWidget::OpenLootingTab(UInventoryComponent* InventoryToLoot)
{
	if (InventoryWidget)
	{
		InventoryWidget->OpenLooting(InventoryToLoot);
	}
}

void UPlayerMainWidget::OpenHUDTab()
{
	TabSwitcher->SetActiveWidget(HUD);
}

void UPlayerMainWidget::OpenInventoryTab()
{
	TabSwitcher->SetActiveWidget(SlotManager);
}
