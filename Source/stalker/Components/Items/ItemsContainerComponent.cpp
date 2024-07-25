// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Library/Items/ItemsFunctionLibrary.h"

UItemsContainerComponent::UItemsContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UItemsContainerComponent::InitializeContainer()
{
	ItemsSlots.SetNum(Columns * Capacity);
	AddStartingData();
}

void UItemsContainerComponent::AddStartingData()
{
	for (auto ItemToAdd : StartingData)
	{
		if (!ItemToAdd.IsValid()) continue;

		for (int i = 0; i < ItemToAdd.Quantity; i++)
		{
			FItemData ItemData;
			ItemData.ItemRow = ItemToAdd.ItemRow;
			ItemData.ItemParams = ItemToAdd.ItemParams;
		
			auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemData);
			if (!ItemObject) continue;

			if (!TryAddItem(ItemObject))
			{
				ItemObject->MarkAsGarbage();
			}
		}
	}
}

bool UItemsContainerComponent::FindAvailablePlace(UItemObject* ItemObject)
{
	if (ItemObject)
	{
		uint32 Index;
		if (CanAddItem(ItemObject, Index))
		{
			AddItemAt(ItemObject, Index);
			return true;
		}
	}
	return false;
}

bool UItemsContainerComponent::TryAddItem(UItemObject* ItemObject)
{
	if (ItemObject)
	{
		uint32 Index;
		if (CanAddItem(ItemObject, Index))
		{
			AddItemAt(ItemObject, Index);
			return true;
		}
	}
	return false;
}

void UItemsContainerComponent::StackItemAt(const UItemObject* ItemObject, uint32 Index)
{
	check(ItemObject);

	if (CanStackAtIndex(ItemObject, Index))
	{
		uint32 ItemId = ItemsSlots[Index];
		if (auto FoundItem = FindItemById(ItemId))
		{
			if (FoundItem->IsSimilar(ItemObject) && FoundItem->IsStackable())
			{
				FoundItem->AddAmount(ItemObject->GetItemParams().Amount);
			}
		}
	}
}

void UItemsContainerComponent::AddItemAt(UItemObject* ItemObject, uint32 Index)
{
	check(ItemObject);
	
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};
	
	if (!ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Add(ItemObject);
	}
	FillRoom(ItemsSlots, ItemObject->GetItemId(), Tile, ItemSize, Columns);
	UpdateItemsMap();
}

void UItemsContainerComponent::DragItem(const UItemObject* ItemObject)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (ItemsSlots[i] == ItemObject->GetItemId())
		{
			ItemsSlots[i] = 0;
		}
	}
}

void UItemsContainerComponent::RemoveItem(UItemObject* ItemObject)
{
	check(ItemObject);
	
	if (ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Remove(ItemObject);
	}
	UpdateItemsMap();
}

void UItemsContainerComponent::UpdateItemsMap()
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
	OnItemsContainerUpdated.Broadcast();
}

bool UItemsContainerComponent::CanStackAtIndex(const UItemObject* ItemObject, uint32 RoomIndex)
{
	bool bResult = false;
	if (ItemObject)
	{
		uint32 ItemId = ItemsSlots[RoomIndex];
		if (auto FoundItem = FindItemById(ItemId))
		{
			bResult = FoundItem->IsSimilar(ItemObject) && FoundItem->IsStackable();
		}
	}
	return bResult;
}

bool UItemsContainerComponent::CanAddItem(const UItemObject* ItemObject, uint32& FindIndex)
{
	bool bResult = false;
	if (ItemObject)
	{
		auto ItemSize = ItemObject->GetItemSize();
		if (Columns >= ItemSize.X)
		{
			bResult = FindAvailableRoom(ItemObject, FindIndex) & ItemObject->GetItemTag().MatchesAny(CategoryTags);
		}
	}
	return bResult;
}

bool UItemsContainerComponent::FindAvailableRoom(const UItemObject* ItemObject, uint32& FindIndex)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (CheckRoom(ItemObject, (uint32)i))
		{
			FindIndex = i;
			return true;
		}
	}
	return false;
}

bool UItemsContainerComponent::CheckRoom(const UItemObject* ItemObject, uint32 Index)
{
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};

	return IsRoomValid(ItemsSlots, Tile, ItemSize, Columns);
}

UItemObject* UItemsContainerComponent::FindItemById(uint32 ItemId) const
{
	UItemObject* FoundItem = nullptr;
	for (auto EachItem : ItemsContainer)
	{
		if (EachItem->GetItemId() == ItemId)
		{
			FoundItem = EachItem;
		}
	}
	return FoundItem;
}

FIntPoint UItemsContainerComponent::TileFromIndex(uint32 Index, uint8 Width)
{
	return {(int)Index % Width, (int)Index / Width};
}

uint32 UItemsContainerComponent::IndexFromTile(const FIntPoint& Tile, int Width)
{
	return Tile.X + Tile.Y * Width;
}

void UItemsContainerComponent::FillRoom(TArray<uint32>& Slots, uint32 ItemId, const FIntPoint& Tile,
                                        const FIntPoint& ItemSize, uint8 Columns)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			Slots[IndexFromTile({x, y}, Columns)] = ItemId;
		}
	}
}

bool UItemsContainerComponent::IsRoomValid(TArray<uint32>& Slots, const FIntPoint& Tile, const FIntPoint& ItemSize,
                                           uint8 Columns)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			if (IsItemSizeValid({x, y}, Columns))
			{
				if (IsTileFilled(Slots, IndexFromTile({x, y}, Columns)))
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

bool UItemsContainerComponent::IsItemSizeValid(const FIntPoint& ItemSize, uint8 Columns)
{
	return ItemSize.X >= 0 && ItemSize.Y >= 0 && ItemSize.X < Columns;
}

bool UItemsContainerComponent::IsTileFilled(const TArray<uint32>& Slots, uint32 Index)
{
	return Slots[Index] != 0;
}
