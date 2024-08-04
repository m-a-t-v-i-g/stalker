// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerManagerWidget.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Inventory/CharacterInventoryComponent.h"
#include "UI/Inventory/CharacterEquipmentWidget.h"
#include "UI/Inventory/EquipmentSlotWidget.h"
#include "UI/Inventory/InventoryWidget.h"
#include "UI/Inventory/ItemDragDropOperation.h"
#include "UI/Inventory/ItemsContainerGridWidget.h"

FReply UPlayerManagerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

bool UPlayerManagerWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	bool bResult = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (!DragDropOperation->bWasSuccessful)
		{
			DragDropOperation->ReverseDragDropOperation();
		}
	}
	return bResult;
}

void UPlayerManagerWidget::InitializeManager(UStalkerAbilityComponent* AbilityComp,
                                             UCharacterInventoryComponent* CharInventoryComp)
{
	OwnAbilityComponent = AbilityComp;
	OwnInventoryComponent = CharInventoryComp;

	check(OwnAbilityComponent.IsValid() && OwnInventoryComponent.IsValid());
	
	Inventory->InitializeInventory(OwnInventoryComponent.Get());
	Inventory->GetItemsGrid()->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerManagerWidget::OnOwnInventoryItemDoubleClick);
	
	Equipment->InitializeCharacterEquipment(OwnInventoryComponent.Get());
	for (auto EachSlotWidget : Equipment->GetAllSlotWidgets())
	{
		EachSlotWidget->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerManagerWidget::OnOwnEquippedItemDoubleClick);
	}

	ClearTabs();
}

void UPlayerManagerWidget::StartLooting(UItemsContainerComponent* LootItemsContainer)
{
	LootingItemsContainer = LootItemsContainer;
	check(LootingItemsContainer.IsValid());
	
	Looting->InitializeInventory(LootingItemsContainer.Get());
	Looting->GetItemsGrid()->OnItemWidgetDoubleClick.AddUObject(this, &UPlayerManagerWidget::OnLootingItemDoubleClick);
	
	ActivateTab(EPlayerManagerTab::Looting);
}

void UPlayerManagerWidget::StartUpgrading()
{
	ActivateTab(EPlayerManagerTab::Upgrading);
}

void UPlayerManagerWidget::ClearTabs()
{
	ActivateTab(EPlayerManagerTab::Inventory);
	
	LootingItemsContainer.Reset();
}

void UPlayerManagerWidget::ActivateTab(EPlayerManagerTab TabToActivate)
{
	ActiveTab = TabToActivate;
	switch (ActiveTab)
	{
	case EPlayerManagerTab::Inventory:
		TabSwitcher->SetActiveWidgetIndex(0);
		break;
	case EPlayerManagerTab::Looting:
		TabSwitcher->SetActiveWidget(Looting);
		break;
	case EPlayerManagerTab::Upgrading:
		TabSwitcher->SetActiveWidget(Upgrading);
		break;
	default: break;
	}
}

void UPlayerManagerWidget::OnOwnInventoryItemDoubleClick(UItemObject* ItemObject)
{
	switch (ActiveTab)
	{
	case EPlayerManagerTab::Inventory:
		OwnInventoryComponent->UseItem(ItemObject);
		break;
	case EPlayerManagerTab::Looting:
		OwnInventoryComponent->MoveItemToOtherContainer(ItemObject, LootingItemsContainer.Get());
		break;
	case EPlayerManagerTab::Upgrading:
		break;
	default: break;
	}
}

void UPlayerManagerWidget::OnOwnEquippedItemDoubleClick(const FString& SlotName)
{
	if (OwnInventoryComponent.IsValid())
	{
		OwnInventoryComponent->UnEquipSlot(SlotName, true);
	}
}

void UPlayerManagerWidget::OnLootingItemDoubleClick(UItemObject* ItemObject)
{
	if (LootingItemsContainer.IsValid())
	{
		LootingItemsContainer->MoveItemToOtherContainer(ItemObject, OwnInventoryComponent.Get());
	}
}
