// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerGridWidget.h"
#include "InteractiveItemWidget.h"
#include "ItemDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Items/ItemsContainerComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Player/PlayerHUD.h"
#include "Slate/SlateBrushAsset.h"

int32 UItemsContainerGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                             const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                             int32 LayerId, const FWidgetStyle& InWidgetStyle,
                                             bool bParentEnabled) const
{
	FPaintContext Context {AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled};
	if (UWidgetBlueprintLibrary::IsDragDropping() && bDrawDropLocation)
	{
		if (auto DragDropOperation = Cast<UItemDragDropOperation>(UWidgetBlueprintLibrary::GetDragDroppingContent()))
		{
			USlateBrushAsset* SlateBrush = Cast<USlateBrushAsset>(USlateBrushAsset::StaticClass()->GetDefaultObject());
			FLinearColor SlotColor {1.0f, 0.0f, 0.0f, 0.35f};
			
			uint32 RoomIndex = UItemsContainerComponent::IndexFromTile(DraggedTile, ItemsContainerRef->GetColumns());
			if (auto ItemObject = DragDropOperation->GetPayload<UItemObject>())
			{
				if (ItemsContainerRef.IsValid() && ItemsContainerRef->CheckRoom(ItemObject, RoomIndex) ||
					ItemsContainerRef->CanStackAtIndex(ItemObject, RoomIndex))
				{
					SlotColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.35f); 
				}
			}
			UWidgetBlueprintLibrary::DrawBox(Context, FVector2D(DraggedTile) * APlayerHUD::TileSize,
											 FVector2D(APlayerHUD::TileSize, APlayerHUD::TileSize), SlateBrush, SlotColor);
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
	bDrawDropLocation = true;
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
}

void UItemsContainerGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent,
                                                  UDragDropOperation* InOperation)
{
	bDrawDropLocation = false;
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
				DragDropOperation->CompleteDragDropOperation();
				ItemsContainerRef->AddItemAt(Payload, RoomIndex);
			}
			else if (ItemsContainerRef->CanStackAtIndex(Payload, RoomIndex))
			{
				ItemsContainerRef->StackItemAt(Payload, RoomIndex);
				DragDropOperation->CompleteDragDropOperation();
			}
			else
			{
				DragDropOperation->ReverseDragDropOperation();
			}
		}
	}
	bDrawDropLocation = false;
	return true;
}

void UItemsContainerGridWidget::SetupContainerGrid(UItemsContainerComponent* OwnContainerComp)
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnItemsContainerUpdated.RemoveAll(this);
	}
	
	ItemsContainerRef = OwnContainerComp;
	ItemsContainerRef->OnItemsContainerUpdated.AddUObject(this, &UItemsContainerGridWidget::OnItemsContainerUpdated);
	
	SetupSize();

	OnItemsContainerUpdated();
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

		if (UInteractiveItemWidget* ItemWidget = CreateWidget<UInteractiveItemWidget>(
			this, APlayerHUD::StaticInteractiveItemWidgetClass))
		{
			ItemWidget->InitItemWidget(ItemObject->GetItemTag(), ItemObject, ItemObject->GetItemSize());
			ItemWidget->OnBeginDragDropOperation.BindUObject(this, &UItemsContainerGridWidget::OnBeginDragOperation);
			ItemWidget->OnCompleteDragDropOperation.BindUObject(this, &UItemsContainerGridWidget::OnCompleteDragOperation);
			ItemWidget->OnReverseDragDropOperation.BindUObject(this, &UItemsContainerGridWidget::OnReverseDragOperation);

			FVector2D WidgetPosition = {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize};
			if (UCanvasPanelSlot* CanvasPanelSlot = GridCanvas->AddChildToCanvas(ItemWidget))
			{
				CanvasPanelSlot->SetAutoSize(true);
				CanvasPanelSlot->SetPosition(WidgetPosition);
			}
		}
	}
}

void UItemsContainerGridWidget::OnBeginDragOperation(UItemObject* DraggedItem)
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->DragItem(DraggedItem);
	}
}

void UItemsContainerGridWidget::OnCompleteDragOperation(UItemObject* DraggedItem)
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->RemoveItem(DraggedItem);
	}
}

void UItemsContainerGridWidget::OnReverseDragOperation(UItemObject* DraggedItem)
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->TryAddItem(DraggedItem);
	}
}

void UItemsContainerGridWidget::SetupSize()
{
	uint16 Width = ItemsContainerRef->GetColumns() * APlayerHUD::TileSize;
	GridSizeBox->SetWidthOverride(Width);
}

bool UItemsContainerGridWidget::IsMouseOnTile(float MousePosition)
{
	if (FGenericPlatformMath::Fmod(MousePosition, APlayerHUD::TileSize) > APlayerHUD::TileSize / 2.0f)
	{
		return true;
	}
	return false;
}
