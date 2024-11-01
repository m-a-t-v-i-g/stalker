// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlotWidget.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EquipmentSlot.h"
#include "InteractiveObjects/ItemSystem/ItemObject.h"
#include "Inventory/ItemDragDropOperation.h"
#include "Inventory/ItemWidget.h"
#include "Player/PlayerHUD.h"

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			if (OwnSlot.IsValid() && OwnSlot->CanEquipItem(Payload))
			{
				if (OwnEquipment->EquipSlot(OwnSlot->GetSlotName(), Payload, false))
				{
					DragDropOperation->bWasSuccessful = true;
				}
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UEquipmentSlotWidget::SetupEquipmentSlot(UCharacterInventoryComponent* CharInventoryComp)
{
	OwnEquipment = CharInventoryComp;
	OwnSlot = CharInventoryComp->FindEquipmentSlot(SlotName);
	
	if (OwnSlot.IsValid())
	{
		OwnSlot->OnSlotChanged.AddUObject(this, &UEquipmentSlotWidget::OnSlotChanged);

		if (OwnSlot->IsEquipped())
		{
			OnSlotChanged(OwnSlot->GetBoundObject(), true, OwnSlot->IsEquipped());
		}
	}
}

void UEquipmentSlotWidget::OnSlotChanged(UItemObject* BoundObject, bool bModified, bool bEquipped)
{
	SlotCanvas->ClearChildren();
	
	if (!bEquipped) return;
	
	if (auto ItemObject = Cast<UItemObject>(BoundObject))
	{
		if (UItemWidget* ItemWidget = CreateWidget<UItemWidget>(
			this, APlayerHUD::StaticItemWidgetClass))
		{
			ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
			ItemWidget->OnDoubleClick.BindUObject(this, &UEquipmentSlotWidget::OnDoubleClick);
			ItemWidget->OnReverseDragDropOperation.BindUObject(this, &UEquipmentSlotWidget::OnReverseDragOperation);
			ItemWidget->OnNotifyDropOperation.BindUObject(this, &UEquipmentSlotWidget::OnCompleteDragOperation);
			
			if (bVerticalSlot)
			{
				ItemWidget->RotateItem();
			}
			
			if (UCanvasPanelSlot* CanvasPanelSlot = SlotCanvas->AddChildToCanvas(ItemWidget))
			{
				FVector2D ItemSize = ItemObject->GetItemSize() * APlayerHUD::TileSize;
				CanvasPanelSlot->SetSize(ItemSize);
				CanvasPanelSlot->SetAnchors(FAnchors(0.5f));
				CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
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
		OwnSlot->UpdateSlot(false);
	}
}

void UEquipmentSlotWidget::OnCompleteDragOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult)
{
	if (OwnEquipment.IsValid())
	{
		OwnEquipment->UnEquipSlot(OwnSlot->GetSlotName(), false);
	}
}
