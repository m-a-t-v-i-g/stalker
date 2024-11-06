// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainer.h"
#include "ItemObject.h"
#include "Items/ItemsFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

void UItemsContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UItemsContainer, ItemsContainer, COND_OwnerOnly);
}

void UItemsContainer::AddStartingData()
{
	for (auto ItemData : StartingData)
	{
		if (!ItemData.IsValid())
		{
			continue;
		}

		if (CanAddItem(ItemData.Definition))
		{
			if (auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(GetWorld(), ItemData.Definition, ItemData.PredictedData))
			{
				AddItem(ItemObject);
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

bool UItemsContainer::StackItem(UItemObject* SourceItem, const UItemObject* TargetItem)
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
		
			UItemsFunctionLibrary::DestroyItemObject(SourceItem);
			return true;
		}
	}
	return false;
}

bool UItemsContainer::AddItem(UItemObject* ItemObject)
{
	if (ItemObject && !ItemsContainer.Contains(ItemObject))
	{
		if (CanAddItem(ItemObject->ItemDefinition))
		{
			ItemObject->SetCollected();
			ItemsContainer.Add(ItemObject);
			OnContainerUpdated.Broadcast(FUpdatedContainerData(ItemObject, nullptr));
			return true;
		}
	}
	return false;
}

void UItemsContainer::SplitItem(UItemObject* ItemObject)
{
	
}

bool UItemsContainer::RemoveItem(UItemObject* ItemObject)
{
	if (ItemObject && ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Remove(ItemObject);
		OnContainerUpdated.Broadcast(FUpdatedContainerData(nullptr, ItemObject));
		return true;
	}
	return false;
}

bool UItemsContainer::SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount)
{
	if (ItemObject && Amount > 0)
	{
		uint16 ItemAmount = ItemObject->GetItemInstance()->Amount;
		if (ItemAmount - Amount > 0)
		{
			ItemObject->RemoveAmount(Amount);
		}
		else
		{
			RemoveItem(ItemObject);
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
	
	if (ItemObject->GetItemInstance()->Amount > ItemObject->GetStackAmount())
	{
		if (auto StackableItem = OtherContainer->FindAvailableStack(ItemObject))
		{
			StackableItem->AddAmount(ItemObject->GetItemInstance()->Amount);
		}
		else if (auto NewItemObject = UItemsFunctionLibrary::GenerateItemObject(GetWorld(), ItemObject))
		{
			if (OtherContainer->AddItem(NewItemObject))
			{
				NewItemObject->SetAmount(NewItemObject->GetStackAmount());
			}
		}
		ItemObject->RemoveAmount(ItemObject->GetStackAmount());
	}
	else
	{
		if (OtherContainer->FindAvailablePlace(ItemObject))
		{
			RemoveItem(ItemObject);
		}
	}
}

bool UItemsContainer::CanAddItem(const UItemDefinition* ItemDefinition) const
{
	return ItemDefinition->Tag.MatchesAny(CategoryTags);
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

void UItemsContainer::OnRep_ItemsContainer(TArray<UItemObject*> PrevContainer)
{
	UItemObject* AddedItem = nullptr;
	UItemObject* RemovedItem = nullptr;
	
	for (UItemObject* Item : ItemsContainer)
	{
		if (PrevContainer.Contains(Item))
		{
			continue;
		}
		
		AddedItem = Item;
		break;
	}
	
	for (UItemObject* Item : PrevContainer)
	{
		if (ItemsContainer.Contains(Item))
		{
			continue;
		}
		
		RemovedItem = Item;
		break;
	}

	OnContainerUpdated.Broadcast(FUpdatedContainerData(AddedItem, RemovedItem));
}
