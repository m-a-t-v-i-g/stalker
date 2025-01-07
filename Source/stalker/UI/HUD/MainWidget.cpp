// Fill out your copyright notice in the Description page of Project Settings.

#include "MainWidget.h"
#include "AbilitySystemComponent.h"
#include "EquipmentComponent.h"
#include "InventoryComponent.h"
#include "InventoryManagerComponent.h"
#include "InventoryManagerWidget.h"
#include "Components/NamedSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"

void UMainWidget::InitializeGameWidget(UInventoryManagerComponent* InventoryManagerComp)
{
	OwnInventoryManager = InventoryManagerComp;
	
	HUD->InitializeHUDWidget();
}

void UMainWidget::ConnectCharacterPart(const FCharacterHUDInitData& HUDInitData)
{
	CharAbilityComponent = HUDInitData.AbilitySystemComponent;
	CharInventoryComponent = HUDInitData.InventoryComponent;
	CharEquipmentComponent = HUDInitData.EquipmentComponent;

	HUD->ConnectCharacterHUD(HUDInitData);
}

void UMainWidget::DisconnectCharacterPart()
{
	HUD->DisconnectCharacterHUD();
	
	CharAbilityComponent.Reset();
	CharInventoryComponent.Reset();
	CharEquipmentComponent.Reset();
}

void UMainWidget::SetupAndOpenOwnInventory()
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

void UMainWidget::ActivateSlotManager()
{
	TabSwitcher->SetActiveWidget(SlotManager);
}
