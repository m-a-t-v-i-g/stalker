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

void UItemsContainer::SetupItemsContainer(FGameplayTagContainer ContainerTags)
{
	CategoryTags = ContainerTags;
}

void UItemsContainer::AddStartingData()
{
	for (const FItemStartingData& ItemData : StartingData)
	{
		if (!ItemData.IsValid())
		{
			continue;
		}

		if (CanAddItem(ItemData.Definition))
		{
			UItemPredictedData* PredictedData = ItemData.bUsePredictedData ? ItemData.PredictedData : nullptr;
			if (UItemObject* ItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemData.Definition, PredictedData))
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
		if (UItemObject* StackableItem = FindAvailableStack(ItemObject))
		{
			if (StackItem(ItemObject, StackableItem))
			{
				return true;
			}
		}

		if (CanAddItem(ItemObject->GetDefinition()))
		{
			AddItem(ItemObject);
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

void UItemsContainer::SplitItem(UItemObject* ItemObject)
{
	if (!ItemObject || !CanAddItem(ItemObject->GetDefinition()))
	{
		return;
	}

	if (ItemObject->GetAmount() > 1)
	{
		if (UItemObject* NewItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemObject))
		{
			NewItemObject->SetAmount(NewItemObject->GetStackAmount()); // TODO
			AddItem(NewItemObject);
		}
	}
}

void UItemsContainer::AddItem(UItemObject* ItemObject)
{
	if (!CanAddItem(ItemObject->GetDefinition()))
	{
		return;
	}

	if (ItemObject && !Items.Contains(ItemObject))
	{
		ItemObject->SetCollected(this);
		Items.Add(ItemObject);
		OnContainerChangeDelegate.Broadcast(FItemsContainerChangeData(TArray{ItemObject}, {}));
	}
}

void UItemsContainer::RemoveItem(UItemObject* ItemObject)
{
	if (ItemObject && Items.Contains(ItemObject))
	{
		Items.Remove(ItemObject);
		OnContainerChangeDelegate.Broadcast(FItemsContainerChangeData({}, TArray{ItemObject}));
	}
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

bool UItemsContainer::CanAddItem(const UItemDefinition* ItemDefinition) const
{
	return ItemDefinition->Tag.MatchesAny(CategoryTags);
}

bool UItemsContainer::Contains(const UItemObject* ItemObject) const
{
	return Items.Contains(ItemObject);
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

void UItemsContainer::OnRep_Items(TArray<UItemObject*> PrevContainer)
{
	TArray<UItemObject*> AddedItems;
	TArray<UItemObject*> RemovedItems;
	
	for (UItemObject* Item : Items)
	{
		if (PrevContainer.Contains(Item))
		{
			continue;
		}

		if (Item)
		{
			Item->SetCollected(this);
		}

		AddedItems.AddUnique(Item);
	}
	
	for (UItemObject* Item : PrevContainer)
	{
		if (Items.Contains(Item))
		{
			continue;
		}
		
		RemovedItems.Remove(Item);
	}

	OnContainerChangeDelegate.Broadcast(FItemsContainerChangeData(AddedItems, RemovedItems));
}
