// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlotWidget.h"
#include "ItemObject.h"
#include "StalkerHUD.h"
#include "Character/CharacterInventoryComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EquipmentSlot.h"
#include "Inventory/ItemDragDropOperation.h"
#include "Inventory/ItemWidget.h"

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			if (EquipmentSlotRef.IsValid() && SlotContainerRef.IsValid())
			{
				if (IsSlotOccupied())
				{
					if (auto SlotContainer = Cast<ISlotContainerInterface>(SlotContainerRef))
					{
						SlotContainer->UnequipAndEquipSlot(EquipmentSlotRef->GetSlotName(), Payload);
					}
				}
				else if (IsSlotEmpty())
				{
					if (auto SlotContainer = Cast<ISlotContainerInterface>(SlotContainerRef))
					{
						if (SlotContainer->CanEquipItemAtSlot(EquipmentSlotRef->GetSlotName(), Payload))
						{
							SlotContainer->EquipSlot(EquipmentSlotRef->GetSlotName(), Payload);
						}
					}
				}
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UEquipmentSlotWidget::SetupEquipmentSlot(UObject* SlotContainerReference)
{
	SlotContainerRef = SlotContainerReference;

	if (SlotContainerRef.IsValid())
	{
		if (auto SlotContainer = Cast<ISlotContainerInterface>(SlotContainerRef))
		{
			EquipmentSlotRef = SlotContainer->FindEquipmentSlot(SlotName);

			if (EquipmentSlotRef.IsValid())
			{
				EquipmentSlotRef->OnSlotChanged.AddUObject(this, &UEquipmentSlotWidget::OnSlotChanged);
				UpdateCanvas();
			}
		}
	}
}

void UEquipmentSlotWidget::ClearEquipmentSlot()
{
	SlotCanvas->ClearChildren();

	if (SlotContainerRef.IsValid())
	{
		SlotContainerRef.Reset();
	}

	if (EquipmentSlotRef.IsValid())
	{
		EquipmentSlotRef->OnSlotChanged.RemoveAll(this);
		EquipmentSlotRef.Reset();
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

void UEquipmentSlotWidget::OnSlotChanged(UItemObject* BoundObject, bool bModified, bool bEquipped)
{
	UpdateCanvas();
}

void UEquipmentSlotWidget::UpdateCanvas()
{
	SlotCanvas->ClearChildren();
	
	if (!EquipmentSlotRef.IsValid() || !EquipmentSlotRef->IsEquipped())
	{
		return;
	}

	CreateItemWidget(EquipmentSlotRef->GetBoundObject());
}

void UEquipmentSlotWidget::OnDoubleClick(UItemObject* ClickedItem)
{
	OnItemWidgetDoubleClick.Broadcast(SlotName);
}

void UEquipmentSlotWidget::OnDragItem(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                      UDragDropOperation* InOperation)
{
	if (!EquipmentSlotRef.IsValid() || !SlotContainerRef.IsValid())
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
	if (!EquipmentSlotRef.IsValid() || !SlotContainerRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		OnDropItem(InOperation);
	}
}

void UEquipmentSlotWidget::OnDropItem(UDragDropOperation* InOperation)
{
	if (!EquipmentSlotRef.IsValid() || !SlotContainerRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.RemoveAll(this);
		DragDropOperation->OnDrop.RemoveAll(this);

		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			switch (DragDropOperation->DragDropOperationResult)
			{
			case EDragDropOperationResult::Failed:
				{
				}
				break;
			case EDragDropOperationResult::Completed:
				{
				}
				break;
			default: break;
			}
		}
	}
}

UItemWidget* UEquipmentSlotWidget::CreateItemWidget(UItemObject* ItemObject)
{
	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(this, AStalkerHUD::StaticItemWidgetClass);
	if (ItemWidget)
	{
		ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
		ItemWidget->OnDoubleClick.BindUObject(this, &UEquipmentSlotWidget::OnDoubleClick);
		ItemWidget->OnDragItem.BindUObject(this, &UEquipmentSlotWidget::OnDragItem);
			
		if (bVerticalSlot)
		{
			ItemWidget->RotateItem();
		}
			
		if (UCanvasPanelSlot* CanvasPanelSlot = SlotCanvas->AddChildToCanvas(ItemWidget))
		{
			FVector2D ItemSize = ItemObject->GetItemSize() * AStalkerHUD::TileSize;
			CanvasPanelSlot->SetSize(ItemSize);
			CanvasPanelSlot->SetAnchors(FAnchors(0.5f));
			CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		}
		else
		{
			ItemWidget->MarkAsGarbage();
		}
	}
	return ItemWidget;
}
