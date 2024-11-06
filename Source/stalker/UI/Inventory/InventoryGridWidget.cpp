// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryGridWidget.h"
#include "ItemDragDropOperation.h"
#include "ItemObject.h"
#include "ItemWidget.h"
#include "StalkerHUD.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/InventoryComponent.h"
#include "Components/ItemsContainer.h"

void UInventoryGridWidget::SetupContainerGrid(UInventoryComponent* InventoryComp)
{
	ClearContainerGrid();
	
	InventoryComponentRef = InventoryComp;

	if (InventoryComponentRef.IsValid())
	{
		ItemsSlots.SetNum(Columns * 50);

		ItemsContainerRef = InventoryComp->GetItemsContainer();

		if (ItemsContainerRef.IsValid())
		{
			for (UItemObject* ItemObject : ItemsContainerRef->GetItems())
			{
				OnContainerUpdated(FUpdatedContainerData(ItemObject, nullptr));
			}
			
			ItemsContainerRef->OnContainerUpdated.AddUObject(this, &UInventoryGridWidget::OnContainerUpdated);
		}
	}
}

void UInventoryGridWidget::ClearContainerGrid()
{
	ClearChildrenItems();

	if (InventoryComponentRef.IsValid())
	{
		InventoryComponentRef.Reset();
	}
	
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnContainerUpdated.RemoveAll(this);
		ItemsContainerRef.Reset();
	}
}

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                        int32 LayerId,
                                        const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (ItemsContainerRef.IsValid())
	{
		FPaintContext Context {AllottedGeometry, MyCullingRect, OutDrawElements, 100, InWidgetStyle, bParentEnabled};
		if (!UWidgetBlueprintLibrary::IsDragDropping() && HoveredData.HasValidData())
		{
			UWidgetBlueprintLibrary::DrawBox(Context, HoveredData.Tile * AStalkerHUD::TileSize, {
				                                 HoveredData.Size.X * AStalkerHUD::TileSize,
				                                 HoveredData.Size.Y * AStalkerHUD::TileSize
			                                 }, GridFillingBrush, GridHighlightColor);
		}
	}
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void UInventoryGridWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

bool UInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                            UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			DraggedData.Tile = GetTileFromMousePosition(GetCachedGeometry(), InDragDropEvent.GetScreenSpacePosition(), Payload);
		}
	}
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			if (ItemsContainerRef.IsValid() && InventoryComponentRef.IsValid())
			{
				uint32 StackableRoom = IndexFromTile(
					GetTileFromMousePosition(InGeometry, InDragDropEvent.GetScreenSpacePosition()), Columns);
				uint32 PlacedRoom = IndexFromTile(DraggedData.Tile, Columns);
				
				if (IsStackableRoom(Payload, StackableRoom))
				{
					uint32 ItemId = ItemsSlots[StackableRoom];
					auto RoomItem = ItemsContainerRef->FindItemById(ItemId);
				
					if (ItemsContainerRef->Contains(Payload))
					{
						ClearRoom(Payload->GetItemId()); // TODO: возможно будут траблы при перемещении в другой контейнер
					}
				
					InventoryComponentRef->ServerStackItem(Payload->GetItemId(), RoomItem->GetItemId());
					DragDropOperation->bWasSuccessful = true;
				}
				else if (IsAvailableRoom(Payload, PlacedRoom) && ItemsContainerRef->Contains(Payload))
				{
					const FIntPoint Tile = TileFromIndex(PlacedRoom, Columns);
					const FIntPoint ItemSize = {Tile.X + (Payload->GetItemSize().X - 1), Tile.Y + (Payload->GetItemSize().Y - 1)};

					FillRoom(Payload->GetItemId(), DraggedData.Tile, ItemSize, Columns);
					DragDropOperation->bWasSuccessful = true;
				}
				else if (!ItemsContainerRef->Contains(Payload))
				{
					InventoryComponentRef->ServerAddItem(Payload->GetItemId());
					DragDropOperation->bWasSuccessful = true;
				}
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UInventoryGridWidget::ClearChildrenItems()
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

void UInventoryGridWidget::UpdateItemsMap()
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

void UInventoryGridWidget::UpdateGrid()
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

		CreateItemWidget(ItemObject, {Tile.X * AStalkerHUD::TileSize, Tile.Y * AStalkerHUD::TileSize});
	}
}

void UInventoryGridWidget::OnContainerUpdated(const FUpdatedContainerData& UpdatedData)
{
	UItemObject* ItemObject;
	
	if (IsValid(UpdatedData.AddedItem))
	{
		ItemObject = UpdatedData.AddedItem;
		
		if (!ItemsMap.Contains(ItemObject->GetItemId()))
		{
			bool bFound;
			uint32 RoomIndex = FindAvailableRoom(ItemObject, bFound);

			if (bFound)
			{
				const FIntPoint Tile = TileFromIndex(RoomIndex, Columns);
				const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};
			
				FillRoom(ItemObject->GetItemId(), Tile, ItemSize, Columns);
				CreateItemWidget(ItemObject, {Tile.X * AStalkerHUD::TileSize, Tile.Y * AStalkerHUD::TileSize});
				UpdateItemsMap();
			}
		}
	}
	else if (IsValid(UpdatedData.RemovedItem))
	{
		ItemObject = UpdatedData.RemovedItem;
		
		if (ItemsMap.Contains(ItemObject->GetItemId()))
		{
			ClearRoom(ItemObject->GetItemId());
			UpdateGrid();
		}
	}
}

void UInventoryGridWidget::OnItemMouseEnter(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent,
                                            UItemObject* HoverItem)
{
	HoveredData.bHighlightItem = true;
	HoveredData.ItemRef = HoverItem;
	HoveredData.Tile = *ItemsMap.Find(HoverItem->GetItemId());
	HoveredData.Size = HoverItem->GetItemSize();
}

void UInventoryGridWidget::OnItemMouseLeave()
{
	HoveredData.Clear();
}

void UInventoryGridWidget::OnDragItem(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                      UDragDropOperation* InOperation)
{
	if (!ItemsContainerRef.IsValid() || !InventoryComponentRef.IsValid())
	{
		return;
	}
	
	HoveredData.Clear();
	DraggedData.Clear();
	
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.AddDynamic(this, &UInventoryGridWidget::OnDragItemCancelled);
		DragDropOperation->OnDrop.AddDynamic(this, &UInventoryGridWidget::OnDropItem);
		
		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			ClearRoom(Payload->GetItemId());
			DraggedData.SourceTile = *ItemsMap.Find(Payload->GetItemId());
		}
	}
}

void UInventoryGridWidget::OnDragItemCancelled(UDragDropOperation* InOperation)
{
	if (!ItemsContainerRef.IsValid() || !InventoryComponentRef.IsValid())
	{
		return;
	}
	
	OnDropItem(InOperation);
}

void UInventoryGridWidget::OnDropItem(UDragDropOperation* InOperation)
{
	if (!ItemsContainerRef.IsValid() || !InventoryComponentRef.IsValid())
	{
		return;
	}
	
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.RemoveAll(this);
		DragDropOperation->OnDrop.RemoveAll(this);

		if (!DragDropOperation->bWasSuccessful || DragDropOperation->bTryRecoveryItem)
		{
			if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
			{
				if (ItemsContainerRef->Contains(Payload))
				{
					const FIntPoint Tile = DraggedData.SourceTile;
					const FIntPoint ItemSize = {
						Tile.X + (Payload->GetItemSize().X - 1), Tile.Y + (Payload->GetItemSize().Y - 1)
					};
					FillRoom(Payload->GetItemId(), Tile, ItemSize, Columns);
				}
			}
		}
	}
	
	UpdateGrid();
	
	HoveredData.Clear();
	DraggedData.Clear();
}

UItemWidget* UInventoryGridWidget::CreateItemWidget(UItemObject* ItemObject, const FVector2D& PositionOnGrid)
{
	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(this, AStalkerHUD::StaticItemWidgetClass);
	if (ItemWidget)
	{
		ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
			
		if (UCanvasPanelSlot* ItemSlot = GridCanvas->AddChildToCanvas(ItemWidget))
		{
			ItemWidget->OnMouseEnter.BindUObject(this, &UInventoryGridWidget::OnItemMouseEnter);
			ItemWidget->OnMouseLeave.BindUObject(this, &UInventoryGridWidget::OnItemMouseLeave);
			ItemWidget->OnDragItem.BindUObject(this, &UInventoryGridWidget::OnDragItem);
			
			FVector2D WidgetPosition = PositionOnGrid;
			ItemSlot->SetPosition(WidgetPosition);
			ItemSlot->SetAutoSize(true);
		}
	}
	return ItemWidget;
}

uint32 UInventoryGridWidget::FindAvailableRoom(const UItemObject* ItemObject, bool& bFound)
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

void UInventoryGridWidget::FillRoom(uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			ItemsSlots[IndexFromTile({x, y}, Width)] = ItemId;
		}
	}
}

void UInventoryGridWidget::ClearRoom(uint32 ItemId)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (ItemsSlots[i] == ItemId)
		{
			ItemsSlots[i] = 0;
		}
	}
}

bool UInventoryGridWidget::IsStackableRoom(const UItemObject* ItemObject, uint32 Index)
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

bool UInventoryGridWidget::IsAvailableRoom(const UItemObject* ItemObject, uint32 Index)
{
	return CheckRoom(ItemObject, Index);
}

bool UInventoryGridWidget::CheckRoom(const UItemObject* ItemObject, uint32 Index)
{
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};

	return IsRoomValid(Tile, ItemSize, Columns);
}

bool UInventoryGridWidget::IsRoomValid(const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width)
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

bool UInventoryGridWidget::IsItemSizeValid(const FIntPoint& ItemSize, uint8 Width)
{
	return ItemSize.X >= 0 && ItemSize.Y >= 0 && ItemSize.X < Width;
}

bool UInventoryGridWidget::IsTileFilled(uint32 Index)
{
	return ItemsSlots[Index] != 0;
}

FIntPoint UInventoryGridWidget::TileFromIndex(uint32 Index, uint8 Width)
{
	return {(int)Index % Width, (int)Index / Width};
}

uint32 UInventoryGridWidget::IndexFromTile(const FIntPoint& Tile, uint8 Width)
{
	return Tile.X + Tile.Y * Width;
}

bool UInventoryGridWidget::IsMouseOnTile(float MousePosition)
{
	if (FGenericPlatformMath::Fmod(MousePosition, AStalkerHUD::TileSize) > AStalkerHUD::TileSize / 2.0f)
	{
		return true;
	}
	return false;
}

FIntPoint UInventoryGridWidget::GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition)
{
	FVector2D MousePosition = InGeometry.AbsoluteToLocal(ScreenSpacePosition);

	bool bPosRight = IsMouseOnTile(MousePosition.X);
	bool bPosDown = IsMouseOnTile(MousePosition.Y);

	int8 SelectX = bPosRight ? 1 : 0;
	int8 SelectY = bPosDown ? 1 : 0;

	FIntPoint MousePosOnTile (MousePosition.X / AStalkerHUD::TileSize, MousePosition.Y / AStalkerHUD::TileSize);
	FIntPoint Dimension (SelectX / 2, SelectY / 2);
	return MousePosOnTile - Dimension;
}

FIntPoint UInventoryGridWidget::GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition,
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
	FIntPoint Dimension (MousePosition.X / AStalkerHUD::TileSize, MousePosition.Y / AStalkerHUD::TileSize);
	return Dimension - MakeIntPoint;
}
