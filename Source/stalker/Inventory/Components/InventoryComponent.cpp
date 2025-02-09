// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "ItemObject.h"
#include "ItemsContainer.h"
#include "Inventory/InventorySystemCore.h"

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (ItemsContainerRef)
		{
			ItemsContainerRef->AddStartingData();
		}
	}
}

void UInventoryComponent::SetItemsContainer(UItemsContainer* InItemsContainer)
{
	ItemsContainerRef = InItemsContainer;
}

void UInventoryComponent::FindAvailablePlace(UItemObject* ItemObject)
{
	if (HasAuthority())
	{
		UInventorySystemCore::Container_FindAvailablePlace(ItemsContainerRef, ItemObject);
	}
}

void UInventoryComponent::StackItem(UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (HasAuthority())
	{
		UInventorySystemCore::Container_StackItem(ItemsContainerRef, SourceItem, TargetItem);
	}
}

void UInventoryComponent::AddItem(UItemObject* ItemObject)
{
	if (HasAuthority())
	{
		UInventorySystemCore::Container_AddItem(ItemsContainerRef, ItemObject);
	}
}

void UInventoryComponent::SplitItem(UItemObject* ItemObject)
{
	if (HasAuthority())
	{
		UInventorySystemCore::Container_SplitItem(ItemsContainerRef, ItemObject);
	}
}

void UInventoryComponent::RemoveItem(UItemObject* ItemObject)
{
	if (HasAuthority())
	{
		UInventorySystemCore::Container_RemoveItem(ItemsContainerRef, ItemObject);
	}
}

void UInventoryComponent::SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount)
{
	if (HasAuthority())
	{
		UInventorySystemCore::Container_SubtractOrRemoveItem(ItemsContainerRef, ItemObject, Amount);
	}
}

bool UInventoryComponent::CanAddItem(const UItemDefinition* ItemDefinition) const
{
	if (ItemsContainerRef)
	{
		return ItemsContainerRef->CanAddItem(ItemDefinition);
	}
	return false;
}

UItemObject* UInventoryComponent::FindItemById(uint32 ItemId) const
{
	if (ItemsContainerRef)
	{
		return ItemsContainerRef->FindItemById(ItemId);
	}
	return nullptr;
}

UItemObject* UInventoryComponent::FindItemByDefinition(const UItemDefinition* Definition) const
{
	if (ItemsContainerRef)
	{
		return ItemsContainerRef->FindItemByDefinition(Definition);
	}
	return nullptr;
}
