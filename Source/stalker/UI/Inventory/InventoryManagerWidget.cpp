// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerWidget.h"
#include "CharacterEquipmentWidget.h"
#include "EquipmentComponent.h"
#include "EquipmentSlotWidget.h"
#include "InventoryComponent.h"
#include "InventoryGridWidget.h"
#include "InventoryManagerComponent.h"
#include "InventoryWidget.h"
#include "StalkerHUD.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/WidgetSwitcher.h"

FReply UInventoryManagerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UInventoryManagerWidget::OpenInventory(UOrganicAbilityComponent* AbilityComp, UInventoryComponent* InventoryComp,
                                            UEquipmentComponent* EquipmentComp, UInventoryManagerComponent* InventoryManager)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = InventoryComp;
	OwnEquipmentComponent = EquipmentComp;
	OwnInventoryManager = InventoryManager;

	check(OwnAbilityComponent.IsValid());
	check(OwnInventoryComponent.IsValid());
	check(OwnEquipmentComponent.IsValid());
	check(OwnInventoryManager.IsValid());

	Inventory->SetupInventory(OwnInventoryComponent.Get(), OwnInventoryManager.Get());
	
	if (UInventoryGridWidget* GridWidget = Inventory->GetInventoryGridWidget())
	{
		GridWidget->OnItemWidgetDoubleClick.AddUObject(this, &UInventoryManagerWidget::OnOwnInventoryItemDoubleClick);
	}
	
	Equipment->SetupCharacterEquipment(OwnAbilityComponent.Get(), OwnEquipmentComponent.Get(), OwnInventoryManager.Get());

	for (UEquipmentSlotWidget* SlotWidget : Equipment->GetAllSlots())
	{
		SlotWidget->OnItemWidgetDoubleClick.AddUObject(this, &UInventoryManagerWidget::OnOwnEquippedItemDoubleClick);
	}
}

void UInventoryManagerWidget::CloseInventory()
{
	OwnAbilityComponent.Reset();
	OwnInventoryComponent.Reset();
	OwnEquipmentComponent.Reset();
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

	Looting->SetupInventory(LootingInventory.Get(), OwnInventoryManager.Get());
	
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
	
	Looting->ClearInventory();
	
	if (UInventoryGridWidget* GridWidget = Looting->GetInventoryGridWidget())
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
			OwnInventoryManager->ServerTryEquipItem(ItemObject);
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

void UInventoryManagerWidget::OnOwnEquippedItemDoubleClick(UEquipmentSlot* EquipmentSlot)
{
	if (OwnInventoryComponent.IsValid())
	{
		OwnInventoryManager->ServerMoveItemFromEquipmentSlot(EquipmentSlot);
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
