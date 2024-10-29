// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerGridWidget.h"

#include "ItemDragDropOperation.h"
#include "ItemObject.h"
#include "ItemWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ItemsContainer.h"
#include "Player/PlayerHUD.h"

void UContainerGridWidget::SetupContainerGrid(UItemsContainer* OwnContainer)
{
	Tiles.SetNum(Columns * 255);
	ClearContainerGrid();
	
	ItemsContainerRef = OwnContainer;
	ItemsContainerRef->OnItemAdded.AddUObject(this, &UContainerGridWidget::OnItemAdded);
	ItemsContainerRef->OnItemRemoved.AddUObject(this, &UContainerGridWidget::OnItemRemoved);
}

void UContainerGridWidget::ClearContainerGrid() const
{
	if (ItemsContainerRef.IsValid())
	{
		
	}
	GridCanvas->ClearChildren();
}

int32 UContainerGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                        int32 LayerId,
                                        const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (ItemsContainerRef.IsValid())
	{
		FPaintContext Context {AllottedGeometry, MyCullingRect, OutDrawElements, 100, InWidgetStyle, bParentEnabled};

		if (UWidgetBlueprintLibrary::IsDragDropping())
		{
			UWidgetBlueprintLibrary::DrawBox(Context, DraggedTile * APlayerHUD::TileSize, {
														 1 * APlayerHUD::TileSize,
														 1 * APlayerHUD::TileSize
													 }, GridFillingBrush, GridHighlightColor);
		}
	}
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

bool UContainerGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
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

			FIntPoint MousePosOnTile = FIntPoint(MousePosition.X / APlayerHUD::TileSize, MousePosition.Y / APlayerHUD::TileSize);
			FIntPoint Dimension = FIntPoint(DimensionX / 2, DimensionY / 2);
			DraggedTile = MousePosOnTile - Dimension;
		}
	}
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UContainerGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			if (auto ItemSlot = ItemsMap.Find(Payload))
			{
				uint32 RoomIndex = IndexFromTile(DraggedTile, Columns);
				if (CheckRoom(Payload, RoomIndex))
				{
					FVector2D WidgetPosition = {DraggedTile.X * APlayerHUD::TileSize, DraggedTile.Y * APlayerHUD::TileSize};
					ItemSlot->ItemSlot->SetPosition(WidgetPosition);
					
					const FIntPoint Tile = TileFromIndex(RoomIndex, Columns);
					const FIntPoint ItemSize = {Tile.X + (Payload->GetItemSize().X - 1), Tile.Y + (Payload->GetItemSize().Y - 1)};
					ClearRoom(Payload->GetItemId());
					FillRoom(Payload->GetItemId(), Tile, ItemSize, Columns);
				}
				ItemSlot->ItemWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UContainerGridWidget::OnItemAdded(UItemObject* ItemObject)
{
	if (!ItemObject)
	{
		return;
	}
	
	if (UItemWidget* ItemWidget = CreateWidget<UItemWidget>(this, APlayerHUD::StaticInteractiveItemWidgetClass))
	{
		bool bRoomFound;
		uint32 RoomIndex = FindAvailableRoom(ItemObject, bRoomFound);

		if (bRoomFound)
		{
			if (UCanvasPanelSlot* ItemSlot = GridCanvas->AddChildToCanvas(ItemWidget))
			{
				const FIntPoint Tile = TileFromIndex(RoomIndex, Columns);
				const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};
				FVector2D WidgetPosition = {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize};
				
				ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
				ItemSlot->SetAutoSize(true);
				ItemSlot->SetPosition(WidgetPosition);
				
				FillRoom(ItemObject->GetItemId(), Tile, ItemSize, Columns);
				ItemsMap.Add(ItemObject, FItemWidgetData(ItemWidget, ItemSlot));
			}
		}
		else
		{
			ItemWidget->MarkAsGarbage();
		}
	}
}

void UContainerGridWidget::OnItemRemoved(UItemObject* ItemObject)
{
	if (!ItemObject)
	{
		return;
	}

	ClearRoom(ItemObject->GetItemId());
	ItemsMap.FindAndRemoveChecked(ItemObject);
}

uint32 UContainerGridWidget::FindAvailableRoom(const UItemObject* ItemObject, bool& bFound)
{
	for (int i = 0; i < Tiles.Num(); i++)
	{
		if (CheckRoom(ItemObject, i))
		{
			bFound = true;
			return i;
		}
	}
	bFound = false;
	return 0;
}

void UContainerGridWidget::FillRoom(uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			Tiles[IndexFromTile({x, y}, Width)] = ItemId;
		}
	}
}

void UContainerGridWidget::ClearRoom(uint32 ItemId)
{
	for (int i = 0; i < Tiles.Num(); i++)
	{
		if (Tiles[i] == ItemId)
		{
			Tiles[i] = 0;
		}
	}
}

bool UContainerGridWidget::CheckRoom(const UItemObject* ItemObject, uint32 Index)
{
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};

	return IsRoomValid(Tile, ItemSize, Columns);
}

bool UContainerGridWidget::IsRoomValid(const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			if (IsItemSizeValid({x, y}, Width))
			{
				if (IsTileFilled(IndexFromTile({x, y}, Width)))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

bool UContainerGridWidget::IsItemSizeValid(const FIntPoint& ItemSize, uint8 Width)
{
	return ItemSize.X >= 0 && ItemSize.Y >= 0 && ItemSize.X < Width;
}

bool UContainerGridWidget::IsTileFilled(uint32 Index)
{
	return Tiles[Index] != 0;
}

FIntPoint UContainerGridWidget::TileFromIndex(uint32 Index, uint8 Width)
{
	return {(int)Index % Width, (int)Index / Width};
}

uint32 UContainerGridWidget::IndexFromTile(const FIntPoint& Tile, uint8 Width)
{
	return Tile.X + Tile.Y * Width;
}

bool UContainerGridWidget::IsMouseOnTile(float MousePosition)
{
	if (FGenericPlatformMath::Fmod(MousePosition, APlayerHUD::TileSize) > APlayerHUD::TileSize / 2.0f)
	{
		return true;
	}
	return false;
}
