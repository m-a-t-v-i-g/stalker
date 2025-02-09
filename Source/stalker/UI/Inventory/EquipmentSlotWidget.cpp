// Fill out your copyright notice in the Description page of Project Settings.

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

	if (!EquipmentSlotRef.IsValid() || !EquipmentComponentRef.IsValid())
	{
		return false;
	}

	auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation);
	if (!DragDropOperation)
	{
		return false;
	}

	auto Payload = DragDropOperation->GetPayload<UItemObject>();
	if (!Payload)
	{
		return false;
	}

	DragDropOperation->Target = EquipmentSlotRef;

	check(DragDropOperation->Source.IsValid());
	check(DragDropOperation->Target.IsValid());

	if (DragDropOperation->Source == DragDropOperation->Target)
	{
		return false;
	}
	
	if (!EquipmentComponentRef->CanEquipItemAtSlot(EquipmentSlotRef->GetSlotName(), Payload))
	{
		return false;
	}

	InventoryManagerRef->EquipSlot(EquipmentSlotRef.Get(), DragDropOperation->Source.Get(), Payload);
	return true;
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
				OnSlotUpdated(FEquipmentSlotChangeData(EquipmentSlotRef->GetSlotName(), EquipmentSlotRef->GetBoundObject(), true));
			}

			EquipmentSlotRef->OnSlotDataChange.AddUObject(this, &UEquipmentSlotWidget::OnSlotUpdated);
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
		EquipmentSlotRef->OnSlotDataChange.RemoveAll(this);
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

void UEquipmentSlotWidget::OnItemDrag(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
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
			OnSlotUpdated(FEquipmentSlotChangeData(EquipmentSlotRef->GetSlotName(), Payload, EquipmentSlotRef->IsEquipped()));
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
			ItemWidget->OnDragItem.BindUObject(this, &UEquipmentSlotWidget::OnItemDrag);
			
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
