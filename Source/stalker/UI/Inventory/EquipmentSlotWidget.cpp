// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlotWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Inventory/EquipmentSlot.h"
#include "Interactive/Items/ItemObject.h"
#include "Player/PlayerHUD.h"
#include "UI/Items/InteractiveItemWidget.h"
#include "UI/Items/ItemDragDropOperation.h"

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	bool bResult = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			if (OwnSlot.IsValid() && OwnSlot->CanEquipItem(Payload))
			{
				if (OwnEquipment->EquipSlot(OwnSlot->GetSlotName(), Payload, false))
				{
					DragDropOperation->CompleteDragDropOperation(EDragDropOperationResult::Subtract);
					return bResult;
				}
			}
		}
		DragDropOperation->ReverseDragDropOperation();
	}
	return bResult;
}

void UEquipmentSlotWidget::SetupEquipmentSlot(UCharacterInventoryComponent* CharInventoryComp)
{
	OwnEquipment = CharInventoryComp;
	OwnSlot = CharInventoryComp->FindEquipmentSlotByName(SlotName);
	
	if (OwnSlot.IsValid())
	{
		OwnSlot->OnSlotChanged.AddUObject(this, &UEquipmentSlotWidget::OnSlotChanged);

		if (OwnSlot->IsEquipped())
		{
			OnSlotChanged(OwnSlot->GetBoundObject());
		}
	}
}

void UEquipmentSlotWidget::OnSlotChanged(UItemObject* BoundObject)
{
	SlotCanvas->ClearChildren();
	
	if (OwnSlot.IsValid() && !OwnSlot->IsEquipped()) return;
	
	if (auto ItemObject = Cast<UItemObject>(BoundObject))
	{
		if (UInteractiveItemWidget* ItemWidget = CreateWidget<UInteractiveItemWidget>(
			this, APlayerHUD::StaticInteractiveItemWidgetClass))
		{
			ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
			ItemWidget->OnDoubleClick.BindUObject(this, &UEquipmentSlotWidget::OnDoubleClick);
			ItemWidget->OnReverseDragDropOperation.BindUObject(this, &UEquipmentSlotWidget::OnReverseDragOperation);
			ItemWidget->OnCompleteDragDropOperation.BindUObject(this, &UEquipmentSlotWidget::OnCompleteDragOperation);
			
			if (UCanvasPanelSlot* CanvasPanelSlot = SlotCanvas->AddChildToCanvas(ItemWidget))
			{
				FVector2D ItemSize = ItemObject->GetItemSize() * APlayerHUD::TileSize;
				CanvasPanelSlot->SetSize(ItemSize);
			}
		}
	}
}

void UEquipmentSlotWidget::OnDoubleClick(UItemObject* ClickedItem)
{
	OnItemWidgetDoubleClick.Broadcast(SlotName);
}

void UEquipmentSlotWidget::OnReverseDragOperation(UItemObject* DraggedItem)
{
	if (OwnSlot.IsValid())
	{
		OwnSlot->UpdateSlot();
	}
}

void UEquipmentSlotWidget::OnCompleteDragOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult)
{
	if (OwnEquipment.IsValid())
	{
		OwnEquipment->UnEquipSlot(OwnSlot->GetSlotName(), false);
	}
}
