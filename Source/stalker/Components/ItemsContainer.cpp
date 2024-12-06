// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainer.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UItemsContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UItemsContainer, Items, COND_OwnerOnly);
}

bool UItemsContainer::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;

	for (UItemObject* EachItem : GetItems())
	{
		bReplicateSomething |= Channel->ReplicateSubobject(EachItem, *Bunch, *RepFlags);
		bReplicateSomething |= EachItem->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}

	return bReplicateSomething;
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
			UItemPredictedData* PredictedData = ItemData.bUsePredictedData ? ItemData.PredictedData : nullptr;
			if (auto ItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemData.Definition, PredictedData))
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
			TargetItem->AddAmount(SourceItem->GetAmount());
		
			if (Items.Contains(SourceItem))
			{
				Items.Remove(SourceItem);
			}
			return true;
		}
	}
	return false;
}

bool UItemsContainer::AddItem(UItemObject* ItemObject)
{
	if (ItemObject && !Items.Contains(ItemObject))
	{
		if (CanAddItem(ItemObject->GetDefinition()))
		{
			Items.Add(ItemObject);
			ItemObject->SetCollected();
			OnContainerUpdated.Broadcast(FUpdatedContainerData(ItemObject, nullptr));
			return true;
		}
	}
	return false;
}

void UItemsContainer::SplitItem(UItemObject* ItemObject)
{
	if (!ItemObject || !CanAddItem(ItemObject->GetDefinition()))
	{
		return;
	}

	if (ItemObject->GetAmount() > 1)
	{
		if (auto NewItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemObject))
		{
			NewItemObject->SetAmount(NewItemObject->GetStackAmount()); // TODO
			AddItem(NewItemObject);
		}
	}
}

bool UItemsContainer::RemoveItem(UItemObject* ItemObject)
{
	if (ItemObject && Items.Contains(ItemObject))
	{
		if (Items.Contains(ItemObject))
		{
			Items.Remove(ItemObject);
			OnContainerUpdated.Broadcast(FUpdatedContainerData(nullptr, ItemObject));
			return true;
		}
	}
	return false;
}

bool UItemsContainer::SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount)
{
	if (ItemObject && Amount > 0)
	{
		uint16 ItemAmount = ItemObject->GetAmount();
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
	
	if (ItemObject->GetAmount() > ItemObject->GetStackAmount())
	{
		if (UItemObject* StackableItem = OtherContainer->FindAvailableStack(ItemObject))
		{
			StackableItem->AddAmount(ItemObject->GetStackAmount());
		}
		else if (UItemObject* NewItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemObject))
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
	return Items.Contains(ItemObject);
}

UItemObject* UItemsContainer::FindItemById(uint32 ItemId) const
{
	for (UItemObject* Item : Items)
	{
		if (Item->GetItemId() == ItemId)
		{
			return Item;
		}
	}
	return nullptr;
}

UItemObject* UItemsContainer::FindItemByDefinition(const UItemDefinition* Definition) const
{
	for (UItemObject* Item : Items)
	{
		if (Item->GetDefinition() == Definition)
		{
			return Item;
		}
	}
	return nullptr;
}

UItemObject* UItemsContainer::FindAvailableStack(const UItemObject* ItemObject) const
{
	for (int i = 0; i < Items.Num(); i++)
	{
		auto FoundItem = Items[i];
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

void UItemsContainer::OnRep_Items(TArray<UItemObject*> PrevContainer)
{
	UItemObject* AddedItem = nullptr;
	UItemObject* RemovedItem = nullptr;
	
	for (UItemObject* Item : Items)
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
		if (Items.Contains(Item))
		{
			continue;
		}
		
		RemovedItem = Item;
		break;
	}

	OnContainerUpdated.Broadcast(FUpdatedContainerData(AddedItem, RemovedItem));
}
