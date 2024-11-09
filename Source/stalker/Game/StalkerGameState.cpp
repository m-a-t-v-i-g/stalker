// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/StalkerGameState.h"
#include "ItemObject.h"

void AStalkerGameState::AddItemObject(uint32 ItemId, UItemObject* ItemObject)
{
	if (!WorldItems.Contains(ItemId))
	{
		WorldItems.Add(ItemId, ItemObject);
	}
	else
	{
		// Error log
	}
}

void AStalkerGameState::RemoveItemObject(uint32 ItemId)
{
	if (UItemObject* ItemObject = WorldItems.FindChecked(ItemId))
	{
		WorldItems.Remove(ItemId);
		ItemObject->MarkAsGarbage();
	}
	else
	{
		// Error log
	}
}

UItemObject* AStalkerGameState::GetItemObjectById(uint32 ItemId) const
{
	if (UItemObject* ItemObject = WorldItems.FindChecked(ItemId))
	{
		if (IsValid(ItemObject))
		{
			return ItemObject;
		}
	}
	return nullptr;
}

bool AStalkerGameState::IsItemObjectExist(uint32 ItemId) const
{
	if (UItemObject* ItemObject = WorldItems.FindChecked(ItemId))
	{
		return IsValid(ItemObject);
	}
	return false;
}
