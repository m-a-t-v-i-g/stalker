// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Kismet/KismetSystemLibrary.h"

UItemsContainerComponent::UItemsContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UItemsContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemsContainerSlots.SetNum(Columns * Rows);
}

bool UItemsContainerComponent::FindAvailablePlace(const UItemObject* ItemObject)
{
	uint32 Index;
	if (CanAddItem(ItemObject, Index))
	{
		AddItemAt(ItemObject, Index);
	}
	return false;
}

void UItemsContainerComponent::AddItemAt(const UItemObject* ItemObject, uint32 Index)
{
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->SizeX - 1), Tile.Y + (ItemObject->SizeY - 1)};

	FillRoom(ItemsContainerSlots, ItemObject->ItemId, Tile, ItemSize, Columns);
}

bool UItemsContainerComponent::CanAddItem(const UItemObject* ItemObject, uint32& Index)
{
	if (ItemsContainerSlots.Num() > 0)
	{
		if (FindAvailableRoom(ItemObject, Index))
		{
			return true;
		}
	}
	return false;
}

bool UItemsContainerComponent::FindAvailableRoom(const UItemObject* ItemObject, uint32& Index)
{
	for (int i = 0; i < ItemsContainerSlots.Num(); i++)
	{
		if (CheckRoom(ItemObject, (uint32)i))
		{
			Index = i;
			return true;
		}
	}
	return false;
}

bool UItemsContainerComponent::CheckRoom(const UItemObject* ItemObject, uint32 Index)
{
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->SizeX - 1), Tile.Y + (ItemObject->SizeY - 1)};

	bool bIsRoomAvailable = IsRoomValid(ItemsContainerSlots, Tile, ItemSize, Columns, Rows);
	return bIsRoomAvailable;
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
                                           uint8 Columns, uint8 Rows)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			if (IsTileValid({x, y}, Columns, Rows))
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

bool UItemsContainerComponent::IsTileValid(const FIntPoint& Tile, uint8 Columns, uint8 Rows)
{
	return Tile.X >= 0 && Tile.Y >= 0 && Tile.X < Columns && Tile.Y < Rows;
}

bool UItemsContainerComponent::IsTileFilled(const TArray<uint32>& Slots, uint32 Index)
{
	if (Slots[Index] != 0)
	{
		return true;
	}
	return false;
}
