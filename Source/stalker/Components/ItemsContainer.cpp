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
			if (!AddItem(ItemObject))
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
		if (TargetItem->CanStackItem(SourceItem))
		{
			TargetItem->AddAmount(SourceItem->GetItemInstance()->Amount);
		
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
	if (ItemObject && !ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Add(ItemObject);
		OnItemAdded.Broadcast(ItemObject);
		return true;
	}
	return false;
}

bool UItemsContainer::RemoveItem(UItemObject* ItemObject)
{
	if (ItemObject && ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Remove(ItemObject);
		OnItemRemoved.Broadcast(ItemObject);
		return true;
	}
	return false;
}

bool UItemsContainer::SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount)
{
	if (ItemObject && Amount > 0)
	{
		uint16 ItemAmount = ItemObject->GetItemParams().Amount;
		if (ItemAmount - Amount > 0)
		{
			ItemObject->RemoveAmount(Amount);
		}
		else
		{
			RemoveItem(ItemObject);
			ItemObject->MarkAsGarbage();
		}
	}
	return false;
}

void UItemsContainer::MoveItemToOtherContainer(UItemObject* ItemObject, UItemsContainer* OtherContainer)
{
	if (!ItemObject || !OtherContainer)
	{
		return;
	}
	
	if (ItemObject->GetItemParams().Amount > ItemObject->GetStackAmount())
	{
		ItemObject->RemoveAmount(ItemObject->GetStackAmount());
		
		if (auto NewItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemObject))
		{
			NewItemObject->SetAmount(NewItemObject->GetStackAmount());
			OtherContainer->FindAvailablePlace(NewItemObject);
		}
	}
	else
	{
		OtherContainer->FindAvailablePlace(ItemObject);
		RemoveItem(ItemObject);
	}
}

bool UItemsContainer::Contains(const UItemObject* ItemObject) const
{
	return ItemsContainer.Contains(ItemObject);
}

UItemObject* UItemsContainer::FindItemById(uint32 ItemId) const
{
	UItemObject* FoundItem = nullptr;
	for (auto EachItem : ItemsContainer)
	{
		if (EachItem->GetItemId() == ItemId)
		{
			FoundItem = EachItem;
			break;
		}
	}
	return FoundItem;
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
		
		if (FoundItem->CanStackItem(ItemObject))
		{
			return FoundItem;
		}
	}
	return nullptr;
}
