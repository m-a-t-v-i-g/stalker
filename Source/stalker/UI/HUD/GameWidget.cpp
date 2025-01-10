// Fill out your copyright notice in the Description page of Project Settings.

#include "GameWidget.h"
#include "AbilitySystemComponent.h"
#include "EquipmentComponent.h"
#include "InventoryComponent.h"
#include "InventoryManagerComponent.h"
#include "InventoryManagerWidget.h"
#include "Components/NamedSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"

void UGameWidget::InitializeGameWidget(UInventoryManagerComponent* InventoryManagerComp)
{
	OwnInventoryManager = InventoryManagerComp;
	
	HUD->InitializeHUDWidget();
}

void UGameWidget::ConnectCharacterPart(const FCharacterHUDInitData& HUDInitData)
{
	CharAbilityComponent = HUDInitData.AbilitySystemComponent;
	CharInventoryComponent = HUDInitData.InventoryComponent;
	CharEquipmentComponent = HUDInitData.EquipmentComponent;

	HUD->ConnectCharacterHUD(HUDInitData);
}

void UGameWidget::DisconnectCharacterPart()
{
	HUD->DisconnectCharacterHUD();
	
	CharAbilityComponent.Reset();
	CharInventoryComponent.Reset();
	CharEquipmentComponent.Reset();
}

void UGameWidget::SetupAndOpenOwnInventory()
{
	if (!InventoryManagerWidget)
	{
		InventoryManagerWidget = CreateWidget<UInventoryManagerWidget>(GetOwningPlayer(), AGameHUD::StaticInventoryWidgetClass);
		
		if (InventoryManagerWidget)
		{
			InventoryManagerWidget->OpenInventory(CharAbilityComponent.Get(), CharInventoryComponent.Get(),
			                                      CharEquipmentComponent.Get(), OwnInventoryManager.Get());
			SlotManager->AddChild(InventoryManagerWidget);
		}
	}
}

void UGameWidget::CloseOwnInventory()
{
	if (InventoryManagerWidget)
	{
		InventoryManagerWidget->RemoveFromParent();
		InventoryManagerWidget->CloseInventory();
		InventoryManagerWidget = nullptr;
	}
}

void UGameWidget::OpenEmptyTab()
{
	if (InventoryManagerWidget)
	{
		InventoryManagerWidget->OpenEmpty();
	}
}

void UGameWidget::OpenLootingTab(UInventoryComponent* InventoryToLoot)
{
	if (InventoryManagerWidget)
	{
		InventoryManagerWidget->OpenLooting(InventoryToLoot);
	}
}

void UGameWidget::OpenHUDTab()
{
	TabSwitcher->SetActiveWidget(HUD);
}

void UGameWidget::ActivateSlotManager()
{
	TabSwitcher->SetActiveWidget(SlotManager);
}
