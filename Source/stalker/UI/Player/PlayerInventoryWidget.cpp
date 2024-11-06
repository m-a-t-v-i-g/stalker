// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInventoryWidget.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Inventory/CharacterEquipmentWidget.h"
#include "UI/Inventory/EquipmentSlotWidget.h"
#include "UI/Inventory/InventoryWidget.h"

FReply UPlayerInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UPlayerInventoryWidget::OpenInventory(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = CharInventoryComp;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());

	Inventory->SetupInventory(OwnInventoryComponent.Get());
	Equipment->SetupCharacterEquipment(OwnInventoryComponent.Get());
	
	for (auto EachSlotWidget : Equipment->GetAllSlots())
	{
		EachSlotWidget->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerInventoryWidget::OnOwnEquippedItemDoubleClick);
	}

	ClearTabs();
}

void UPlayerInventoryWidget::CloseInventory()
{
	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());
	
	OwnAbilityComponent.Reset();
	OwnInventoryComponent.Reset();
	
	Inventory->ClearInventory();
}

void UPlayerInventoryWidget::StartLooting(UInventoryComponent* LootItemsContainer)
{
	LootingInventory = LootItemsContainer;
	check(LootingInventory.IsValid());
	
	ActivateTab(EPlayerInventoryTab::Looting);
}

void UPlayerInventoryWidget::StartUpgrading()
{
	ActivateTab(EPlayerInventoryTab::Upgrading);
}

void UPlayerInventoryWidget::ClearTabs()
{
	ActivateTab(EPlayerInventoryTab::Inventory);
	
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
	switch (ActiveTab)
	{
	case EPlayerInventoryTab::Inventory:
		OwnInventoryComponent->UseItem(ItemObject);
		break;
	case EPlayerInventoryTab::Looting:
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

	}
}

void UPlayerInventoryWidget::OnLootingItemDoubleClick(UItemObject* ItemObject)
{
	if (LootingInventory.IsValid())
	{
		
	}
}
