﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlotWidget.h"
#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "ItemDragDropOperation.h"
#include "ItemObject.h"
#include "ItemWidget.h"
#include "GameHUD.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

// TODO: Drag and Drop by tags
bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	check(InventoryManagerRef.IsValid());

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			if (EquipmentComponentRef.IsValid() && EquipmentSlotRef.IsValid())
			{
				if (EquipmentComponentRef->CanEquipItemAtSlot(EquipmentSlotRef->GetSlotName(), Payload))
				{
					InventoryManagerRef->EquipSlot(EquipmentSlotRef.Get(), Payload);
					return true;
				}
			}
		}
	}
	return false;
}

void UEquipmentSlotWidget::SetupEquipmentSlot(UEquipmentComponent* EquipmentComp, UInventoryManagerComponent* InventoryManager)
{
	ClearEquipmentSlot();
	
	EquipmentComponentRef = EquipmentComp;
	InventoryManagerRef = InventoryManager;

	if (EquipmentComponentRef.IsValid() && InventoryManagerRef.IsValid())
	{
		EquipmentSlotRef = EquipmentComp->FindEquipmentSlot(SlotName);

		if (EquipmentSlotRef.IsValid())
		{
			if (EquipmentSlotRef->IsEquipped())
			{
				OnSlotUpdated(FEquipmentSlotChangeData(EquipmentSlotRef->GetBoundObject(), true));
			}

			EquipmentSlotRef->OnSlotChanged.AddUObject(this, &UEquipmentSlotWidget::OnSlotUpdated);
		}
	}
}

void UEquipmentSlotWidget::ClearEquipmentSlot()
{
	SlotCanvas->ClearChildren();

	if (EquipmentComponentRef.IsValid())
	{
		EquipmentComponentRef.Reset();
	}

	if (EquipmentSlotRef.IsValid())
	{
		EquipmentSlotRef->OnSlotChanged.RemoveAll(this);
		EquipmentSlotRef.Reset();
	}

	if (InventoryManagerRef.IsValid())
	{
		InventoryManagerRef.Reset();
	}
}

bool UEquipmentSlotWidget::IsSlotOccupied() const
{
	return EquipmentSlotRef->IsEquipped();
}

bool UEquipmentSlotWidget::IsSlotEmpty() const
{
	return !EquipmentSlotRef->IsEquipped();
}

void UEquipmentSlotWidget::OnSlotUpdated(const FEquipmentSlotChangeData& UpdatedData)
{
	SlotCanvas->ClearChildren();
	
	if (!UpdatedData.bIsEquipped)
	{
		return;
	}

	if (!EquipmentSlotRef.IsValid() || !EquipmentSlotRef->IsEquipped())
	{
		return;
	}

	CreateItemWidget(EquipmentSlotRef->GetBoundObject());
}

void UEquipmentSlotWidget::OnDoubleClick(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent,
                                         UItemObject* ItemObject)
{
	OnItemWidgetDoubleClick.Broadcast(EquipmentSlotRef.Get());
}

void UEquipmentSlotWidget::OnDragItem(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                      UDragDropOperation* InOperation)
{
	if (!EquipmentSlotRef.IsValid() || !EquipmentComponentRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.AddDynamic(this, &UEquipmentSlotWidget::OnDragItemCancelled);
		DragDropOperation->OnDrop.AddDynamic(this, &UEquipmentSlotWidget::OnDropItem);
	}
}

void UEquipmentSlotWidget::OnDragItemCancelled(UDragDropOperation* InOperation)
{
	if (!EquipmentSlotRef.IsValid() || !EquipmentComponentRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.RemoveAll(this);
		DragDropOperation->OnDrop.RemoveAll(this);

		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			OnSlotUpdated(FEquipmentSlotChangeData(Payload, EquipmentSlotRef->IsEquipped()));
		}
	}
}

void UEquipmentSlotWidget::OnDropItem(UDragDropOperation* InOperation)
{
	if (!EquipmentSlotRef.IsValid() || !EquipmentComponentRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.RemoveAll(this);
		DragDropOperation->OnDrop.RemoveAll(this);

		InventoryManagerRef->UnequipSlot(EquipmentSlotRef.Get());
	}
}

UItemWidget* UEquipmentSlotWidget::CreateItemWidget(UItemObject* ItemObject)
{
	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(this, AGameHUD::StaticItemWidgetClass);
	if (ItemWidget)
	{
		ItemWidget->InitItemWidget(EquipmentSlotRef.Get(), ItemObject, ItemObject->GetItemSize());
		
		if (UCanvasPanelSlot* CanvasPanelSlot = SlotCanvas->AddChildToCanvas(ItemWidget))
		{
			ItemWidget->OnDoubleClick.BindUObject(this, &UEquipmentSlotWidget::OnDoubleClick);
			ItemWidget->OnDragItem.BindUObject(this, &UEquipmentSlotWidget::OnDragItem);
			
			if (bVerticalSlot)
			{
				ItemWidget->RotateItem();
			}
			
			FVector2D ItemSize = ItemObject->GetItemSize() * AGameHUD::TileSize;
			CanvasPanelSlot->SetSize(ItemSize);
			CanvasPanelSlot->SetAnchors(FAnchors(0.5f));
			CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		}
	}
	return ItemWidget;
}
