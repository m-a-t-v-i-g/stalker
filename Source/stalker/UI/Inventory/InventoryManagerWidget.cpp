// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerWidget.h"
#include "AbilitySystemComponent.h"
#include "CharacterEquipmentWidget.h"
#include "EquipmentComponent.h"
#include "EquipmentSlotWidget.h"
#include "InventoryComponent.h"
#include "InventoryGridWidget.h"
#include "InventoryManagerComponent.h"
#include "InventoryWidget.h"
#include "Components/WidgetSwitcher.h"

FReply UInventoryManagerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UInventoryManagerWidget::OpenInventory(UAbilitySystemComponent* AbilityComp, UInventoryComponent* InventoryComp,
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
	
	OwnAbilityComponent.Reset();
	OwnInventoryComponent.Reset();
	OwnEquipmentComponent.Reset();
	OwnInventoryManager.Reset();
}

void UInventoryManagerWidget::OpenEmpty()
{
	ActivateTab(EInventoryTab::Inventory);
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
	
	ActivateTab(EInventoryTab::Looting);
}

void UInventoryManagerWidget::OpenUpgrading()
{
	ActivateTab(EInventoryTab::Upgrading);
}

void UInventoryManagerWidget::ClearTabs()
{
	ActivateTab(EInventoryTab::Inventory);
	
	Looting->ClearInventory();
	
	if (UInventoryGridWidget* GridWidget = Looting->GetInventoryGridWidget())
	{
		GridWidget->OnItemWidgetDoubleClick.RemoveAll(this);
	}
	
	LootingInventory.Reset();
}

void UInventoryManagerWidget::ActivateTab(EInventoryTab TabToActivate)
{
	ActiveTab = TabToActivate;
	switch (ActiveTab)
	{
	case EInventoryTab::Inventory:
		TabSwitcher->SetActiveWidgetIndex(0);
		break;
	case EInventoryTab::Looting:
		TabSwitcher->SetActiveWidget(Looting);
		break;
	case EInventoryTab::Upgrading:
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
	case EInventoryTab::Inventory:
		{
			OwnInventoryManager->ServerTryEquipItem(ItemObject);
		}
		break;
	case EInventoryTab::Looting:
		{
			OwnInventoryManager->ServerMoveItemToOtherContainer(OwnInventoryComponent->GetItemsContainer(),
			                                                    LootingInventory->GetItemsContainer(), ItemObject);
		}
		break;
	case EInventoryTab::Upgrading:
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
