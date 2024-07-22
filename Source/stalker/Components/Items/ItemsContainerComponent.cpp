// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Library/Items/ItemsFunctionLibrary.h"

UItemsContainerComponent::UItemsContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UItemsContainerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//ItemsContainerSlots.SetNum(Columns * Rows);
}

void UItemsContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		AddStartingData();
	}
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

void UItemsContainerComponent::AddItemAt(UItemObject* ItemObject, uint32 Index)
{
	check(ItemObject);
	
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};
	
	if (!ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Add(ItemObject);
	}
	FillRoom(ItemsContainerSlots, ItemObject->GetItemId(), Tile, ItemSize, Columns);
	OnItemAddedToContainer.Broadcast(ItemObject, Tile);
}

void UItemsContainerComponent::RemoveItem(UItemObject* ItemObject)
{
	check(ItemObject);
	
	if (ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Remove(ItemObject);
	}
	ResizeSlots(ItemObject, true);
	OnItemRemovedFromContainer.Broadcast(ItemObject);
}

void UItemsContainerComponent::ResizeSlots(const UItemObject* ItemObject, bool bSubtract)
{
	if (bSubtract)
	{
		ItemsContainerSlots.SetNum(ItemsContainerSlots.Num() - Columns * ItemObject->GetItemSize().Y);
	}
	else
	{
		ItemsContainerSlots.SetNum(ItemsContainerSlots.Num() + Columns * ItemObject->GetItemSize().Y);
	}
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
	ResizeSlots(ItemObject, false);
	for (int i = 0; i < ItemsContainerSlots.Num(); i++)
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

	return IsRoomValid(ItemsContainerSlots, Tile, ItemSize, Columns);
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
