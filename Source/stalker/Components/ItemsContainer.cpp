// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainer.h"
#include "ItemObject.h"
#include "Items/ItemsFunctionLibrary.h"

void UItemsContainer::AddStartingData()
{
	for (auto ItemData : StartingData)
	{
		if (!ItemData.IsValid())
		{
			continue;
		}
		
		if (auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemData.Definition, ItemData.PredictedData))
		{
			if (!FindAvailablePlace(ItemObject))
			{
				ItemObject->MarkAsGarbage();
			}
		}
	}

	StartingData.Empty();
}

bool UItemsContainer::FindAvailablePlace(UItemObject* ItemObject)
{
	if (ItemObject)
	{
		if (auto StackableItem = FindAvailableStack(ItemObject))
		{
			if (StackItem(ItemObject, StackableItem))
			{
				return true;
			}
		}
		if (AddItem(ItemObject))
		{
			return true;
		}
	}
	return false;
}

bool UItemsContainer::StackItem(UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (SourceItem && TargetItem)
	{
		if (TargetItem->IsStackable() && TargetItem->IsSimilar(SourceItem))
		{
			TargetItem->AddAmount(SourceItem->GetItemParams().Amount);
		
			if (ItemsContainer.Contains(SourceItem))
			{
				ItemsContainer.Remove(SourceItem);
			}
		
			SourceItem->MarkAsGarbage();
			return true;
		}
	}
	
	return false;
}

bool UItemsContainer::AddItem(UItemObject* ItemObject)
{
	if (!ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Add(ItemObject);
		OnItemAdded.Broadcast(ItemObject);
		return true;
	}
	
	return false;
}

UItemObject* UItemsContainer::FindAvailableStack(const UItemObject* ItemObject) const
{
	for (int i = 0; i < ItemsContainer.Num(); i++)
	{
		auto FoundItem = ItemsContainer[i];
		if (!FoundItem)
		{
			continue;
		}
		
		if (FoundItem != ItemObject && FoundItem->IsStackable() && FoundItem->IsSimilar(ItemObject))
		{
			return FoundItem;
		}
	}
	return nullptr;
}
