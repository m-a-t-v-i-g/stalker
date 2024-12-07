// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerWidget.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/StalkerHUD.h"
#include "Inventory/EquipmentSlotWidget.h"
#include "Inventory/InventoryGridWidget.h"
#include "Inventory/InventoryWidget.h"
#include "Inventory/Character/CharacterEquipmentWidget.h"
#include "Player/InventoryManagerComponent.h"

FReply UInventoryManagerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UInventoryManagerWidget::OpenInventory(UOrganicAbilityComponent* AbilityComp,
                                           UCharacterInventoryComponent* CharInventoryComp,
                                           UInventoryManagerComponent* PlayerInventoryManager)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = CharInventoryComp;
	OwnInventoryManager = PlayerInventoryManager;

	check(OwnAbilityComponent.IsValid());
	check(OwnInventoryComponent.IsValid());
	check(OwnInventoryManager.IsValid());

	Inventory->SetupInventory(OwnInventoryManager.Get(), OwnInventoryComponent->GetItemsContainer());
	
	if (UInventoryGridWidget* GridWidget = Inventory->GetInventoryGridWidget())
	{
		GridWidget->OnItemWidgetDoubleClick.AddUObject(this, &UInventoryManagerWidget::OnOwnInventoryItemDoubleClick);
	}
	
	Equipment->SetupCharacterEquipment(OwnInventoryComponent.Get());

	for (UEquipmentSlotWidget* EachSlotWidget : Equipment->GetAllSlots())
	{
		EachSlotWidget->OnItemWidgetDoubleClick.AddUObject(this, &UInventoryManagerWidget::OnOwnEquippedItemDoubleClick);
	}
}

void UInventoryManagerWidget::CloseInventory()
{
	OwnAbilityComponent.Reset();
	OwnInventoryComponent.Reset();
	OwnInventoryManager.Reset();

	Inventory->ClearInventory();

	if (UInventoryGridWidget* GridWidget = Inventory->GetInventoryGridWidget())
	{
		GridWidget->OnItemWidgetDoubleClick.RemoveAll(this);
	}

	Equipment->ClearCharacterEquipment();
	
	for (UEquipmentSlotWidget* EachSlotWidget : Equipment->GetAllSlots())
	{
		EachSlotWidget->OnItemWidgetDoubleClick.RemoveAll(this);
	}

	ClearTabs();
}

void UInventoryManagerWidget::OpenEmpty()
{
	ActivateTab(EPlayerInventoryTab::Inventory);
}

void UInventoryManagerWidget::OpenLooting(UInventoryComponent* LootItemsContainer)
{
	LootingInventory = LootItemsContainer;
	check(LootingInventory.IsValid());

	Looting->SetupInventory(OwnInventoryManager.Get(), LootingInventory->GetItemsContainer());
	
	if (UInventoryGridWidget* LootingGrid = Looting->GetInventoryGridWidget())
	{
		LootingGrid->OnItemWidgetDoubleClick.AddUObject(this, &UInventoryManagerWidget::OnLootingInventoryItemDoubleClick);
	}
	
	ActivateTab(EPlayerInventoryTab::Looting);
}

void UInventoryManagerWidget::OpenUpgrading()
{
	ActivateTab(EPlayerInventoryTab::Upgrading);
}

void UInventoryManagerWidget::ClearTabs()
{
	ActivateTab(EPlayerInventoryTab::Inventory);
	
	Inventory->ClearInventory();
	
	if (UInventoryGridWidget* GridWidget = Inventory->GetInventoryGridWidget())
	{
		GridWidget->OnItemWidgetDoubleClick.RemoveAll(this);
	}
	
	LootingInventory.Reset();
}

void UInventoryManagerWidget::ActivateTab(EPlayerInventoryTab TabToActivate)
{
	ActiveTab = TabToActivate;
	switch (ActiveTab)
	{
	case EPlayerInventoryTab::Inventory:
		TabSwitcher->SetActiveWidgetIndex(0);
		break;
	case EPlayerInventoryTab::Looting:
		TabSwitcher->SetActiveWidget(Looting);
		break;
	case EPlayerInventoryTab::Upgrading:
		TabSwitcher->SetActiveWidget(Upgrading);
		break;
	default: break;
	}
}

void UInventoryManagerWidget::OnOwnInventoryItemDoubleClick(UItemObject* ItemObject)
{
	if (!OwnInventoryManager.IsValid())
	{
		return;
	}
	
	switch (ActiveTab)
	{
	case EPlayerInventoryTab::Inventory:
		{
			OwnInventoryComponent->TryEquipItem(ItemObject);
		}
		break;
	case EPlayerInventoryTab::Looting:
		{
			OwnInventoryManager->ServerMoveItemToOtherContainer(OwnInventoryComponent->GetItemsContainer(),
			                                                    LootingInventory->GetItemsContainer(), ItemObject);
		}
		break;
	case EPlayerInventoryTab::Upgrading:
		break;
	default: break;
	}
}

void UInventoryManagerWidget::OnOwnEquippedItemDoubleClick(const FString& SlotName)
{
	if (OwnInventoryComponent.IsValid())
	{
		OwnInventoryComponent->ServerMoveItemFromSlot(SlotName);
	}
}

void UInventoryManagerWidget::OnLootingInventoryItemDoubleClick(UItemObject* ItemObject)
{
	if (LootingInventory.IsValid())
	{
		OwnInventoryManager->ServerMoveItemToOtherContainer(LootingInventory->GetItemsContainer(),
		                                                    OwnInventoryComponent->GetItemsContainer(), ItemObject);
	}
}
