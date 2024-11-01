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
	ClearContainerGrid();
	
	ItemsSlots.SetNum(Columns * 50);
	
	ItemsContainerRef = OwnContainer;
	ItemsContainerRef->OnItemAdded.AddUObject(this, &UContainerGridWidget::OnItemAdded);
	ItemsContainerRef->OnItemRemoved.AddUObject(this, &UContainerGridWidget::OnItemRemoved);

	for (auto ItemObject : ItemsContainerRef->GetItems())
	{
		OnItemAdded(ItemObject);
	}
}

void UContainerGridWidget::ClearContainerGrid()
{
	ClearChildrenItems();
	
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnItemAdded.RemoveAll(this);
		ItemsContainerRef->OnItemRemoved.RemoveAll(this);
		ItemsContainerRef.Reset();
	}
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
				                                 HoveredData.Size.X * APlayerHUD::TileSize,
				                                 HoveredData.Size.Y * APlayerHUD::TileSize
			                                 }, GridFillingBrush, GridHighlightColor);
		}
	}
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

bool UContainerGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                            UDragDropOperation* InOperation)
{
	DraggedData.Tile = GetTileFromMousePosition(GetCachedGeometry(), InDragDropEvent.GetScreenSpacePosition(),
	                                            DraggedData.ItemRef.Get());
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
			if (IsStackableRoom(Payload, RoomIndex))
			{
				uint32 ItemId = ItemsSlots[RoomIndex];
				auto RoomItem = ItemsContainerRef->FindItemById(ItemId);
				
				if (ItemsContainerRef->Contains(Payload))
				{
					ClearRoom(Payload->GetItemId());
				}
				
				if (ItemsContainerRef->StackItem(Payload, RoomItem)) // TODO: сделать взаимодействие посредством "запросов" через компонент
				{
					UpdateItemsMap();
				}
				
				DragDropOperation->bWasSuccessful = true;
			}
			else if (IsAvailableRoom(Payload, RoomIndex))
			{
				const FIntPoint Tile = TileFromIndex(RoomIndex, Columns);
				const FIntPoint ItemSize = {Tile.X + (Payload->GetItemSize().X - 1), Tile.Y + (Payload->GetItemSize().Y - 1)};
				
				FillRoom(Payload->GetItemId(), DraggedData.Tile, ItemSize, Columns);

				if (!ItemsContainerRef->Contains(Payload))
				{
					ItemsContainerRef->AddItem(Payload); // TODO: сделать взаимодействие посредством "запросов" через компонент
				}
				else
				{
					UpdateGrid();
				}
				
				DragDropOperation->bWasSuccessful = true;
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UContainerGridWidget::ClearChildrenItems()
{
	TArray<UWidget*> Children = GridCanvas->GetAllChildren();
	
	GridCanvas->ClearChildren();

	if (!Children.IsEmpty())
	{
		for (auto Child : Children)
		{
			Child->MarkAsGarbage();
		}
	}
}

void UContainerGridWidget::UpdateItemsMap()
{
	ItemsMap.Empty();
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (ItemsSlots[i] != 0)
		{
			if (!ItemsMap.Contains(ItemsSlots[i]))
			{
				ItemsMap.Add(ItemsSlots[i], TileFromIndex(i, Columns));
			}
		}
	}
}

void UContainerGridWidget::UpdateGrid()
{
	ClearChildrenItems();
	UpdateItemsMap();

	for (auto EachItem : ItemsMap)
	{
		auto ItemId = EachItem.Key;
		FIntPoint Tile = EachItem.Value;

		auto ItemObject = ItemsContainerRef->FindItemById(ItemId);
		
		if (!IsValid(ItemObject))
		{
			continue;
		}

		CreateItemWidget(ItemObject, {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize});
	}
}

void UContainerGridWidget::OnItemAdded(UItemObject* ItemObject)
{
	if (!ItemsMap.Contains(ItemObject->GetItemId()))
	{
		bool bFound;
		uint32 RoomIndex = FindAvailableRoom(ItemObject, bFound);

		if (bFound)
		{
			const FIntPoint Tile = TileFromIndex(RoomIndex, Columns);
			const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};
			
			FillRoom(ItemObject->GetItemId(), Tile, ItemSize, Columns);
			CreateItemWidget(ItemObject, {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize});
			UpdateItemsMap();
		}
	}
}

void UContainerGridWidget::OnItemRemoved(UItemObject* ItemObject)
{
	if (ItemsMap.Contains(ItemObject->GetItemId()))
	{
		ClearRoom(ItemObject->GetItemId());
		UpdateGrid();
	}
}

void UContainerGridWidget::OnItemMouseEnter(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent,
                                            UItemObject* HoverItem)
{
	HoveredData.bHighlightItem = true;
	HoveredData.ItemRef = HoverItem;
	HoveredData.Tile = *ItemsMap.Find(HoverItem->GetItemId());
	HoveredData.Size = HoverItem->GetItemSize();
}

void UContainerGridWidget::OnItemMouseLeave(UItemObject* HoverItem)
{
	HoveredData.Clear();
}

void UContainerGridWidget::OnBeginDragOperation(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UItemObject* ItemObject)
{
	ClearRoom(ItemObject->GetItemId());
	
	HoveredData.Clear();
	DraggedData.Clear();

	DraggedData.ItemRef = ItemObject;
	DraggedData.SourceTile = *ItemsMap.Find(ItemObject->GetItemId());
}

void UContainerGridWidget::OnNotifiedAboutDropOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult)
{
	switch (OperationResult)
	{
	case EDragDropOperationResult::Failed:
		{
			const FIntPoint ItemSize = {
				DraggedData.SourceTile.X + (DraggedItem->GetItemSize().X - 1),
				DraggedData.SourceTile.Y + (DraggedItem->GetItemSize().Y - 1)
			};
			
			FillRoom(DraggedItem->GetItemId(), DraggedData.SourceTile, ItemSize, Columns);
		}
		break;
	case EDragDropOperationResult::Remove:
		{
			if (ItemsContainerRef.IsValid())
			{
				ItemsContainerRef->RemoveItem(DraggedItem);
			}
		}
		break;
	default: break;	
	}
	
	UpdateGrid();
	
	HoveredData.Clear();
	DraggedData.Clear();
}

uint32 UContainerGridWidget::FindAvailableRoom(const UItemObject* ItemObject, bool& bFound)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
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
			ItemsSlots[IndexFromTile({x, y}, Width)] = ItemId;
		}
	}
}

void UContainerGridWidget::ClearRoom(uint32 ItemId)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (ItemsSlots[i] == ItemId)
		{
			ItemsSlots[i] = 0;
		}
	}
}

bool UContainerGridWidget::IsStackableRoom(const UItemObject* ItemObject, uint32 Index)
{
	bool bResult = false;
	if (ItemsSlots.IsValidIndex(Index))
	{
		uint32 ItemId = ItemsSlots[Index];
		auto RoomItem = ItemsContainerRef->FindItemById(ItemId);
		bResult = RoomItem != nullptr && RoomItem->CanStackItem(ItemObject);
	}
	return bResult;
}

bool UContainerGridWidget::IsAvailableRoom(const UItemObject* ItemObject, uint32 Index)
{
	return CheckRoom(ItemObject, Index);
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
	return ItemsSlots[Index] != 0;
}

UItemWidget* UContainerGridWidget::CreateItemWidget(UItemObject* ItemObject, const FVector2D& PositionOnGrid)
{
	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(this, APlayerHUD::StaticItemWidgetClass);
	if (ItemWidget)
	{
		ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
			
		if (UCanvasPanelSlot* ItemSlot = GridCanvas->AddChildToCanvas(ItemWidget))
		{
			ItemWidget->OnMouseEnter.BindUObject(this, &UContainerGridWidget::OnItemMouseEnter);
			ItemWidget->OnMouseLeave.BindUObject(this, &UContainerGridWidget::OnItemMouseLeave);

			ItemWidget->OnBeginDragOperation.BindUObject(this, &UContainerGridWidget::OnBeginDragOperation);
			ItemWidget->OnNotifyDropOperation.BindUObject(this, &UContainerGridWidget::OnNotifiedAboutDropOperation);

			FVector2D WidgetPosition = PositionOnGrid;
			ItemSlot->SetPosition(WidgetPosition);
			ItemSlot->SetAutoSize(true);
		}
		else
		{
			ItemWidget->MarkAsGarbage();
		}
	}
	return ItemWidget;
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

	FIntPoint MousePosOnTile (MousePosition.X / APlayerHUD::TileSize, MousePosition.Y / APlayerHUD::TileSize);
	FIntPoint Dimension (SelectX / 2, SelectY / 2);
	return MousePosOnTile - Dimension;
}

FIntPoint UContainerGridWidget::GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition,
                                                         const UItemObject* ItemObject)
{
	FVector2D MousePosition = InGeometry.AbsoluteToLocal(ScreenSpacePosition);

	bool bPosRight = IsMouseOnTile(MousePosition.X);
	bool bPosDown = IsMouseOnTile(MousePosition.Y);

	int8 SelectX = bPosRight ? 1 : 0;
	int8 SelectY = bPosDown ? 1 : 0;

	int8 DimensionX = FMath::Clamp(ItemObject->GetItemSize().X - SelectX, 0, ItemObject->GetItemSize().X - SelectX);
	int8 DimensionY = FMath::Clamp(ItemObject->GetItemSize().X - SelectX, 0, ItemObject->GetItemSize().Y - SelectY);

	FIntPoint MakeIntPoint (DimensionX / 2, DimensionY / 2);
	FIntPoint Dimension (MousePosition.X / APlayerHUD::TileSize, MousePosition.Y / APlayerHUD::TileSize);
	return Dimension - MakeIntPoint;
}
