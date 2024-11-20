// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInventoryWidget.h"
#include "StalkerHUD.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Inventory/CharacterEquipmentWidget.h"
#include "Inventory/EquipmentSlotWidget.h"
#include "Inventory/InventoryGridWidget.h"
#include "Inventory/InventoryWidget.h"
#include "Player/PlayerInventoryManagerComponent.h"

FReply UPlayerInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UPlayerInventoryWidget::OpenInventory(UOrganicAbilityComponent* AbilityComp,
                                           UCharacterInventoryComponent* CharInventoryComp,
                                           UPlayerInventoryManagerComponent* PlayerInventoryManager)
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
		GridWidget->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerInventoryWidget::OnOwnInventoryItemDoubleClick);
	}
	
	Equipment->SetupCharacterEquipment(OwnInventoryComponent.Get());

	for (UEquipmentSlotWidget* EachSlotWidget : Equipment->GetAllSlots())
	{
		EachSlotWidget->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerInventoryWidget::OnOwnEquippedItemDoubleClick);
	}
}

void UPlayerInventoryWidget::CloseInventory()
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

void UPlayerInventoryWidget::OpenEmpty()
{
	ActivateTab(EPlayerInventoryTab::Inventory);
}

void UPlayerInventoryWidget::OpenLooting(UInventoryComponent* LootItemsContainer)
{
	LootingInventory = LootItemsContainer;
	check(LootingInventory.IsValid());

	Looting->SetupInventory(OwnInventoryManager.Get(), LootingInventory->GetItemsContainer());
	
	if (UInventoryGridWidget* LootingGrid = Looting->GetInventoryGridWidget())
	{
		LootingGrid->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerInventoryWidget::OnLootingInventoryItemDoubleClick);
	}
	
	ActivateTab(EPlayerInventoryTab::Looting);
}

void UPlayerInventoryWidget::OpenUpgrading()
{
	ActivateTab(EPlayerInventoryTab::Upgrading);
}

void UPlayerInventoryWidget::ClearTabs()
{
	ActivateTab(EPlayerInventoryTab::Inventory);
	
	Inventory->ClearInventory();
	
	if (UInventoryGridWidget* GridWidget = Inventory->GetInventoryGridWidget())
	{
		GridWidget->OnItemWidgetDoubleClick.RemoveAll(this);
	}
	
	LootingInventory.Reset();
}

void UPlayerInventoryWidget::ActivateTab(EPlayerInventoryTab TabToActivate)
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

void UPlayerInventoryWidget::OnOwnInventoryItemDoubleClick(UItemObject* ItemObject)
{
	if (!OwnInventoryComponent.IsValid())
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

void UPlayerInventoryWidget::OnOwnEquippedItemDoubleClick(const FString& SlotName)
{
	if (OwnInventoryComponent.IsValid())
	{
		OwnInventoryComponent->ServerMoveItemFromSlot(SlotName);
	}
}

void UPlayerInventoryWidget::OnLootingInventoryItemDoubleClick(UItemObject* ItemObject)
{
	if (LootingInventory.IsValid())
	{
		OwnInventoryManager->ServerMoveItemToOtherContainer(LootingInventory->GetItemsContainer(),
		                                                    OwnInventoryComponent->GetItemsContainer(), ItemObject);
	}
}
