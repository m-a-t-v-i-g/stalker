// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryGridWidget.h"
#include "InventoryComponent.h"
#include "InventoryManagerComponent.h"
#include "ItemDragDropOperation.h"
#include "ItemObject.h"
#include "ItemsContainer.h"
#include "ItemWidget.h"
#include "GameHUD.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UInventoryGridWidget::SetupContainerGrid(UInventoryComponent* InventoryComp, UInventoryManagerComponent* InventoryManager)
{
	ClearContainerGrid();

	InventoryComponentRef = InventoryComp;
	InventoryManagerRef = InventoryManager;

	if (InventoryComponentRef.IsValid() && InventoryManagerRef.IsValid())
	{
		ItemsContainerRef = InventoryComponentRef->GetItemsContainer();

		if (ItemsContainerRef.IsValid())
		{
			ItemsSlots.SetNum(Columns * 50);

			ItemsContainerRef->OnContainerChangeDelegate.AddUObject(this, &UInventoryGridWidget::OnContainerUpdated);
			OnContainerUpdated(FItemsContainerChangeData(ItemsContainerRef->GetItems(), {}));
		}
	}
}

void UInventoryGridWidget::ClearContainerGrid()
{
	ClearChildrenItems();

	if (InventoryManagerRef.IsValid())
	{
		InventoryManagerRef.Reset();
	}
	
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnContainerChangeDelegate.RemoveAll(this);
		ItemsContainerRef.Reset();
	}
}

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                                        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                                        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (ItemsContainerRef.IsValid())
	{
		FPaintContext Context {AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled};
		if (!UWidgetBlueprintLibrary::IsDragDropping() && HoveredData.HasValidData())
		{
			UWidgetBlueprintLibrary::DrawBox(Context, HoveredData.TilePoint * AGameHUD::TileSize, {
				                                 HoveredData.Size.X * AGameHUD::TileSize,
				                                 HoveredData.Size.Y * AGameHUD::TileSize
			                                 }, GridFillingBrush, GridHighlightColor);
		}
	}
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

bool UInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                            UDragDropOperation* InOperation)
{
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		if (auto Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			DraggedData.Tile = GetTileFromMousePosition(GetCachedGeometry(), InDragDropEvent.GetScreenSpacePosition(), Payload);
		}
	}
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                        UDragDropOperation* InOperation)
{
	check(InventoryManagerRef.IsValid());

	if (!ItemsContainerRef.IsValid() || !InventoryComponentRef.IsValid())
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

	DragDropOperation->Target = ItemsContainerRef;

	check(DragDropOperation->Source.IsValid());
	check(DragDropOperation->Target.IsValid());

	const uint32 StackableRoom = IndexFromTile(GetTileFromMousePosition(InGeometry, InDragDropEvent.GetScreenSpacePosition()), Columns);
	if (IsStackableRoom(Payload, StackableRoom))
	{
		const uint32 ItemId = ItemsSlots[StackableRoom];
		UItemObject* RoomItem = ItemsContainerRef->FindItemById(ItemId);

		InventoryManagerRef->StackItem(ItemsContainerRef.Get(), DragDropOperation->Source.Get(), Payload, RoomItem);
	}
	else if (DragDropOperation->Source == DragDropOperation->Target)
	{
		const uint32 PlacedRoom = IndexFromTile(DraggedData.Tile, Columns);
		if (IsAvailableRoom(Payload, PlacedRoom))
		{
			const FIntPoint Tile = TileFromIndex(PlacedRoom, Columns);
			const FIntPoint ItemSize = {
				Tile.X + (Payload->GetItemSize().X - 1), Tile.Y + (Payload->GetItemSize().Y - 1)
			};

			FillRoom(Payload->GetItemId(), DraggedData.Tile, ItemSize, Columns);
		}
		else
		{
			return false;
		}
	}
	else
	{
		check(!ItemsContainerRef->Contains(Payload));
		InventoryManagerRef->AddItem(ItemsContainerRef.Get(), DragDropOperation->Source.Get(), Payload);
	}
	
	return true;
}

void UInventoryGridWidget::ClearChildrenItems() const
{
	TArray<UWidget*> Children = GridCanvas->GetAllChildren();
	
	GridCanvas->ClearChildren();

	if (!Children.IsEmpty())
	{
		for (UWidget* Child : Children)
		{
			if (auto ItemWidget = Cast<UItemWidget>(Child))
			{
				ItemWidget->ClearItemWidget();
				ItemWidget->OnMouseEnter.Unbind();
				ItemWidget->OnMouseLeave.Unbind();
				ItemWidget->OnDoubleClick.Unbind();
				ItemWidget->OnDragItem.Unbind();
			}
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

	for (auto Pair : ItemsMap)
	{
		uint32 ItemId = Pair.Key;
		FIntPoint Tile = Pair.Value;

		auto ItemObject = ItemsContainerRef->FindItemById(ItemId);
		
		if (!IsValid(ItemObject))
		{
			continue;
		}

		CreateItemWidget(ItemObject, {Tile.X * AGameHUD::TileSize, Tile.Y * AGameHUD::TileSize});
	}
}

void UInventoryGridWidget::OnContainerUpdated(const FItemsContainerChangeData& UpdatedData)
{
	if (!UpdatedData.AddedItems.IsEmpty())
	{
		for (UItemObject* Item : UpdatedData.AddedItems)
		{
			if (!IsValid(Item))
			{
				continue;
			}
			
			if (!ItemsMap.Contains(Item->GetItemId()))
			{
				bool bFound;
				uint32 RoomIndex = FindAvailableRoom(Item, bFound);

				if (bFound)
				{
					const FIntPoint Tile = TileFromIndex(RoomIndex, Columns);
					const FIntPoint ItemSize = {Tile.X + (Item->GetItemSize().X - 1), Tile.Y + (Item->GetItemSize().Y - 1)};
			
					FillRoom(Item->GetItemId(), Tile, ItemSize, Columns);
					CreateItemWidget(Item, {Tile.X * AGameHUD::TileSize, Tile.Y * AGameHUD::TileSize});
				}
			}
		}
		UpdateItemsMap();
	}
	else if (!UpdatedData.RemovedItems.IsEmpty())
	{
		for (UItemObject* Item : UpdatedData.RemovedItems)
		{
			if (!IsValid(Item))
			{
				continue;
			}
			
			if (ItemsMap.Contains(Item->GetItemId()))
			{
				ClearRoom(Item->GetItemId());
				RemoveItemWidget(Item);
			}
		}
		UpdateItemsMap();
	}

	HoveredData.Clear();
}

void UInventoryGridWidget::OnItemMouseEnter(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent,
                                            UItemObject* HoverItem)
{
	HoveredData.bNeedHighlight = true;
	HoveredData.ItemRef = HoverItem;
	HoveredData.TilePoint = *ItemsMap.Find(HoverItem->GetItemId());
	HoveredData.Size = HoverItem->GetItemSize();
}

void UInventoryGridWidget::OnItemMouseLeave(const FPointerEvent& InMouseEvent)
{
	HoveredData.Clear();
}

void UInventoryGridWidget::OnItemDoubleClick(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent,
                                             UItemObject* ItemObject)
{
	OnItemWidgetDoubleClick.Broadcast(ItemObject);
}

void UInventoryGridWidget::OnItemDrag(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                      UDragDropOperation* InOperation)
{
	if (!ItemsContainerRef.IsValid() || !InventoryManagerRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.AddDynamic(this, &UInventoryGridWidget::OnDragItemCancelled);
		DragDropOperation->OnDrop.AddDynamic(this, &UInventoryGridWidget::OnDropItem);

		if (auto Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			ClearRoom(Payload->GetItemId());
			DraggedData.SourceTile = *ItemsMap.Find(Payload->GetItemId());
		}
	}
	
	HoveredData.Clear();
	DraggedData.Clear();
}

void UInventoryGridWidget::OnDragItemCancelled(UDragDropOperation* InOperation)
{
	if (!ItemsContainerRef.IsValid() || !InventoryManagerRef.IsValid())
	{
		return;
	}
		
	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.RemoveAll(this);
		DragDropOperation->OnDrop.RemoveAll(this);

		if (UItemObject* Payload = DragDropOperation->GetPayload<UItemObject>())
		{
			bool bFound;
			uint32 RoomIndex = FindAvailableRoom(Payload, bFound);
			FIntPoint Tile = TileFromIndex(RoomIndex, Columns);

			if (bFound)
			{
				const FIntPoint ItemSize = {
					Tile.X + (Payload->GetItemSize().X - 1), Tile.Y + (Payload->GetItemSize().Y - 1)
				};
				FillRoom(Payload->GetItemId(), Tile, ItemSize, Columns);
			}
		}
	}
		
	UpdateGrid();

	HoveredData.Clear();
	DraggedData.Clear();
}

void UInventoryGridWidget::OnDropItem(UDragDropOperation* InOperation)
{
	if (!ItemsContainerRef.IsValid() || !InventoryManagerRef.IsValid())
	{
		return;
	}

	if (auto DragDropOperation = Cast<UItemDragDropOperation>(InOperation))
	{
		DragDropOperation->OnDragCancelled.RemoveAll(this);
		DragDropOperation->OnDrop.RemoveAll(this);
	}

	UpdateGrid();
}

UItemWidget* UInventoryGridWidget::CreateItemWidget(UItemObject* ItemObject, const FVector2D& PositionOnGrid)
{
	UItemWidget* ItemWidget = CreateWidget<UItemWidget>(this, AGameHUD::StaticItemWidgetClass);
	if (ItemWidget)
	{
		ItemWidget->InitItemWidget(ItemsContainerRef.Get(), ItemObject, ItemObject->GetItemSize());

		if (UCanvasPanelSlot* ItemSlot = GridCanvas->AddChildToCanvas(ItemWidget))
		{
			ItemWidget->OnMouseEnter.BindUObject(this, &UInventoryGridWidget::OnItemMouseEnter);
			ItemWidget->OnMouseLeave.BindUObject(this, &UInventoryGridWidget::OnItemMouseLeave);
			ItemWidget->OnDoubleClick.BindUObject(this, &UInventoryGridWidget::OnItemDoubleClick);
			ItemWidget->OnDragItem.BindUObject(this, &UInventoryGridWidget::OnItemDrag);
			
			FVector2D WidgetPosition = PositionOnGrid;
			ItemSlot->SetPosition(WidgetPosition);
			ItemSlot->SetAutoSize(true);

			ItemWidgetMap.Add(ItemObject->GetItemId(), ItemWidget);
		}
	}
	return ItemWidget;
}

bool UInventoryGridWidget::RemoveItemWidget(UItemObject* ItemObject)
{
	check(ItemObject);
	
	if (UItemWidget** ItemWidgetPtr = ItemWidgetMap.Find(ItemObject->GetItemId()))
	{
		if (UItemWidget* ItemWidget = *ItemWidgetPtr)
		{
			ItemWidget->RemoveFromParent();
			ItemWidgetMap.Remove(ItemObject->GetItemId());
			return true;
		}
	}
	return false;
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
		UItemObject* RoomItem = ItemsContainerRef->FindItemById(ItemId);
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
	if (FGenericPlatformMath::Fmod(MousePosition, AGameHUD::TileSize) > AGameHUD::TileSize / 2.0f)
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

	FIntPoint MousePosOnTile (MousePosition.X / AGameHUD::TileSize, MousePosition.Y / AGameHUD::TileSize);
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
	FIntPoint Dimension (MousePosition.X / AGameHUD::TileSize, MousePosition.Y / AGameHUD::TileSize);
	return Dimension - MakeIntPoint;
}
