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

void UContainerGridWidget::ClearContainerGrid()
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnItemAdded.RemoveAll(this);
		ItemsContainerRef->OnItemRemoved.RemoveAll(this);
		ItemsContainerRef.Reset();
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

		if (!UWidgetBlueprintLibrary::IsDragDropping() && HoveredData.HasValidData())
		{
			UWidgetBlueprintLibrary::DrawBox(Context, HoveredData.Tile * APlayerHUD::TileSize, {
				                                 HoveredData.HoveredItemRef->GetItemSize().X * APlayerHUD::TileSize,
				                                 HoveredData.HoveredItemRef->GetItemSize().Y * APlayerHUD::TileSize
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
		if (UItemObject* Payload = Cast<UItemObject>(DragDropOperation->Payload))
		{
			DraggedData.Tile = GetTileFromMousePosition(GetCachedGeometry(), InDragDropEvent.GetScreenSpacePosition());
			DraggedData.Index = IndexFromTile(DraggedData.Tile, Columns);
			DraggedData.ItemSize = {
				DraggedData.Tile.X + (Payload->GetItemSize().X - 1), DraggedData.Tile.Y + (Payload->GetItemSize().Y - 1)
			};
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
			uint32 RoomIndex = IndexFromTile(DraggedData.Tile, Columns);
			if (CheckRoom(Payload, RoomIndex))
			{
				FIntPoint Tile = DraggedData.Tile;
				FIntPoint ItemSize = DraggedData.ItemSize;
				
				DragDropOperation->CompleteDragDropOperation(EDragDropOperationResult::Remove);
				FillRoom(Payload->GetItemId(), Tile, ItemSize, Columns);
			}
			else
			{
				DragDropOperation->ReverseDragDropOperation();
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
				
				ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
				ItemWidget->OnMouseEnter.BindUObject(this, &UContainerGridWidget::OnItemMouseEnter);
				ItemWidget->OnMouseLeave.BindUObject(this, &UContainerGridWidget::OnItemMouseLeave);
				
				ItemWidget->OnBeginDragDropOperation.BindUObject(this, &UContainerGridWidget::OnBeginDragOperation);
				ItemWidget->OnCompleteDragDropOperation.BindUObject(this, &UContainerGridWidget::OnCompleteDragOperation);
				
				FVector2D WidgetPosition = {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize};
				ItemSlot->SetPosition(WidgetPosition);
				ItemSlot->SetAutoSize(true);
				
				FillRoom(ItemObject->GetItemId(), Tile, ItemSize, Columns);
				ItemsMap.Add(ItemObject, FItemWidgetData(ItemWidget, Tile));
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

	if (auto WidgetData = ItemsMap.Find(ItemObject))
	{
		WidgetData->ItemWidget->OnCompleteDragDropOperation.Unbind();
	}
	
	ClearRoom(ItemObject->GetItemId());
	ItemsMap.FindAndRemoveChecked(ItemObject);
}

void UContainerGridWidget::OnItemMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UItemObject* HoverItem)
{
	HoveredData.bHighlightItem = true;
	HoveredData.HoveredItemRef = HoverItem;
	
	if (auto WidgetData = ItemsMap.Find(HoverItem))
	{
		HoveredData.Tile = WidgetData->Tile;
	}
}

void UContainerGridWidget::OnItemMouseLeave(UItemObject* HoverItem)
{
	HoveredData.Clear();
}

void UContainerGridWidget::OnBeginDragOperation(UItemObject* DraggedItem)
{
	HoveredData.Clear();
	DraggedData.Clear();

	if (auto ItemData = ItemsMap.Find(DraggedItem))
	{
		DraggedData.ItemObject = DraggedItem;
		DraggedData.ItemWidget = ItemData->ItemWidget;
	}
}

void UContainerGridWidget::OnReverseDragOperation(UItemObject* DraggedItem)
{
	if (ItemsContainerRef.IsValid())
	{
		
	}
}

void UContainerGridWidget::OnCompleteDragOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult)
{
	if (ItemsContainerRef.IsValid())
	{
		switch (OperationResult)
		{
		case EDragDropOperationResult::Remove:
			{
				ClearRoom(DraggedItem->GetItemId());
			}
			break;
		case EDragDropOperationResult::Subtract:
			{
				ClearRoom(DraggedItem->GetItemId());
			}
			break;
		default: break;
		}
	}
	
	HoveredData.Clear();
	DraggedData.Clear();
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

void UContainerGridWidget::EmplaceItemOnGrid(const UItemObject* ItemObject)
{
	if (UCanvasPanelSlot* ItemSlot = GridCanvas->AddChildToCanvas(DraggedData.ItemWidget))
	{
		FVector2D WidgetPosition = {DraggedData.Tile.X * APlayerHUD::TileSize, DraggedData.Tile.Y * APlayerHUD::TileSize};
		ItemSlot->SetPosition(WidgetPosition);
		ItemSlot->SetAutoSize(true);
		
		if (auto WidgetData = ItemsMap.Find(ItemObject))
		{
			WidgetData->Tile = DraggedData.Tile;
		}
	}
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

FIntPoint UContainerGridWidget::GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition)
{
	FVector2D MousePosition = InGeometry.AbsoluteToLocal(ScreenSpacePosition);

	bool bPosRight = IsMouseOnTile(MousePosition.X);
	bool bPosDown = IsMouseOnTile(MousePosition.Y);

	int8 SelectX = bPosRight ? 1 : 0;
	int8 SelectY = bPosDown ? 1 : 0;

	FIntPoint MousePosOnTile = FIntPoint(MousePosition.X / APlayerHUD::TileSize,
	                                     MousePosition.Y / APlayerHUD::TileSize);
	FIntPoint Dimension = FIntPoint(SelectX / 2, SelectY / 2);
	return MousePosOnTile - Dimension;
}
