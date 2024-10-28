// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerGridWidget.h"
#include "ItemDragDropOperation.h"
#include "ItemWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ItemsContainerComponent.h"
#include "Components/SizeBox.h"
#include "InteractiveObjects/Items/ItemObject.h"
#include "Player/PlayerHUD.h"

int32 UItemsContainerGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                             const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                             int32 LayerId, const FWidgetStyle& InWidgetStyle,
                                             bool bParentEnabled) const
{
	if (ItemsContainerRef.IsValid())
	{
		FPaintContext Context {AllottedGeometry, MyCullingRect, OutDrawElements, 100, InWidgetStyle, bParentEnabled};
		if (bHighlightItem && HoveredItem.IsValid())
		{
			auto ItemsMap = ItemsContainerRef->GetItemsMap();
			if (auto Tile = ItemsMap.Find(HoveredItem.Get()->GetItemId()))
			{
				UWidgetBlueprintLibrary::DrawBox(Context, *Tile * APlayerHUD::TileSize,
												 FVector2D(HoveredItem->GetItemSize().X * APlayerHUD::TileSize,
														   HoveredItem->GetItemSize().Y * APlayerHUD::TileSize),
												 GridFillingBrush, GridHighlightColor);
			}
		}
	}
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                          bParentEnabled);
}

bool UItemsContainerGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                                 UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		FVector2D MousePosition = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());

		bool bPosRight = IsMouseOnTile(MousePosition.X);
		bool bPosDown = IsMouseOnTile(MousePosition.Y);

		int8 SelectX = bPosRight ? 1 : 0;
		int8 SelectY = bPosDown ? 1 : 0;

		if (UItemObject* Payload = Cast<UItemObject>(DragDropOperation->Payload))
		{
			int8 DimensionX = FMath::Clamp(Payload->GetItemSize().X - SelectX, 0, Payload->GetItemSize().X - SelectX);
			int8 DimensionY = FMath::Clamp(Payload->GetItemSize().X - SelectX, 0, Payload->GetItemSize().Y - SelectY);

			FIntPoint MakeIntPoint = FIntPoint(DimensionX / 2, DimensionY / 2);
			DraggedTile = FIntPoint(MousePosition.X / APlayerHUD::TileSize, MousePosition.Y / APlayerHUD::TileSize) - MakeIntPoint;
		}
	}
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

void UItemsContainerGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                                  UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
}

void UItemsContainerGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent,
                                                  UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

bool UItemsContainerGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                             UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			uint32 RoomIndex = UItemsContainerComponent::IndexFromTile(DraggedTile, ItemsContainerRef->GetColumns());
			if (ItemsContainerRef->CheckRoom(Payload, RoomIndex))
			{
				DragDropOperation->bWasSuccessful = true;
				DragDropOperation->CompleteDragDropOperation(EDragDropOperationResult::Remove);
				
				ItemsContainerRef->AddItemAt(Payload, RoomIndex);
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UItemsContainerGridWidget::SetupContainerGrid(UItemsContainerComponent* OwnContainerComp)
{
	ClearContainerGrid();
	
	ItemsContainerRef = OwnContainerComp;
	ItemsContainerRef->OnItemsContainerUpdated.AddUObject(this, &UItemsContainerGridWidget::OnItemsContainerUpdated);
	
	SetupSize();

	OnItemsContainerUpdated();
}

void UItemsContainerGridWidget::ClearContainerGrid() const
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnItemsContainerUpdated.RemoveAll(this);
	}
	GridCanvas->ClearChildren();
}

void UItemsContainerGridWidget::OnItemsContainerUpdated()
{
	GridCanvas->ClearChildren();
	
	auto ItemsMap = ItemsContainerRef->GetItemsMap();
	for (auto EachItem : ItemsMap)
	{
		auto ItemId = EachItem.Key;
		FIntPoint Tile = EachItem.Value;

		auto ItemObject = ItemsContainerRef->FindItemById(ItemId);
		if (!IsValid(ItemObject)) return;

		if (UItemWidget* ItemWidget = CreateWidget<UItemWidget>(
			this, APlayerHUD::StaticInteractiveItemWidgetClass))
		{
			ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
			ItemWidget->OnMouseEnter.BindUObject(this, &UItemsContainerGridWidget::OnItemMouseEnter);
			ItemWidget->OnMouseLeave.BindUObject(this, &UItemsContainerGridWidget::OnItemMouseLeave);
			ItemWidget->OnDoubleClick.BindUObject(this, &UItemsContainerGridWidget::OnDoubleClick);

			ItemWidget->OnBeginDragDropOperation.BindUObject(this, &UItemsContainerGridWidget::OnBeginDragOperation);
			ItemWidget->OnReverseDragDropOperation.BindUObject(this, &UItemsContainerGridWidget::OnReverseDragOperation);
			ItemWidget->OnCompleteDragDropOperation.BindUObject(this, &UItemsContainerGridWidget::OnCompleteDragOperation);

			FVector2D WidgetPosition = {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize};
			if (UCanvasPanelSlot* CanvasPanelSlot = GridCanvas->AddChildToCanvas(ItemWidget))
			{
				CanvasPanelSlot->SetAutoSize(true);
				CanvasPanelSlot->SetPosition(WidgetPosition);
			}
		}
	}
}

void UItemsContainerGridWidget::SetupSize()
{
	uint16 Width = ItemsContainerRef->GetColumns() * APlayerHUD::TileSize;
	GridSizeBox->SetWidthOverride(Width);
}

void UItemsContainerGridWidget::OnItemMouseEnter(UItemObject* HoverItem)
{
	bHighlightItem = true;
	HoveredItem = HoverItem;
}

void UItemsContainerGridWidget::OnItemMouseLeave(UItemObject* HoverItem)
{
	bHighlightItem = false;
	HoveredItem = HoverItem;
}

void UItemsContainerGridWidget::OnDoubleClick(UItemObject* ClickedItem)
{
	OnItemWidgetDoubleClick.Broadcast(ClickedItem);
}

void UItemsContainerGridWidget::OnBeginDragOperation(UItemObject* DraggedItem)
{
	bHighlightItem = false;
}

void UItemsContainerGridWidget::OnReverseDragOperation(UItemObject* DraggedItem)
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->UpdateItemsMap();
	}
}

void UItemsContainerGridWidget::OnCompleteDragOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult)
{
	if (ItemsContainerRef.IsValid())
	{
		switch (OperationResult)
		{
		case EDragDropOperationResult::Remove:
			ItemsContainerRef->RemoveItem(DraggedItem, false);
			break;
		case EDragDropOperationResult::Subtract:
			ItemsContainerRef->SubtractOrRemoveItem(DraggedItem, 1);
			break;
		default: break;
		}
	}
}

bool UItemsContainerGridWidget::IsMouseOnTile(float MousePosition)
{
	if (FGenericPlatformMath::Fmod(MousePosition, APlayerHUD::TileSize) > APlayerHUD::TileSize / 2.0f)
	{
		return true;
	}
	return false;
}
