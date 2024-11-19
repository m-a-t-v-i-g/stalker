// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "ItemObject.h"
#include "ItemsContainer.h"
#include "ItemSystemCore.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
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

void UInventoryComponent::ServerFindAvailablePlace_Implementation(UItemObject* ItemObject)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->FindAvailablePlace(ItemObject);
	}
}

bool UInventoryComponent::ServerFindAvailablePlace_Validate(UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryComponent::ServerStackItem_Implementation(UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->StackItem(SourceItem, TargetItem);
	}
}

bool UInventoryComponent::ServerStackItem_Validate(UItemObject* SourceItem, UItemObject* TargetItem)
{
	return IsItemObjectValid(SourceItem->GetItemId());
}

void UInventoryComponent::ServerAddItem_Implementation(UItemObject* ItemObject)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->AddItem(ItemObject);
	}
}

bool UInventoryComponent::ServerAddItem_Validate(UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryComponent::ServerSplitItem_Implementation(UItemObject* ItemObject)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->SplitItem(ItemObject);
	}
}

bool UInventoryComponent::ServerSplitItem_Validate(UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryComponent::ServerRemoveItem_Implementation(UItemObject* ItemObject)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->RemoveItem(ItemObject);
	}
}

bool UInventoryComponent::ServerRemoveItem_Validate(UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryComponent::ServerSubtractOrRemoveItem_Implementation(UItemObject* ItemObject, uint16 Amount)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->SubtractOrRemoveItem(ItemObject, Amount);
	}
}

bool UInventoryComponent::ServerSubtractOrRemoveItem_Validate(UItemObject* ItemObject, uint16 Amount)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryComponent::ServerMoveItemToOtherContainer_Implementation(UItemObject* ItemObject, UItemsContainer* OtherContainer)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->MoveItemToOtherContainer(ItemObject, OtherContainer);
	}
}

bool UInventoryComponent::ServerMoveItemToOtherContainer_Validate(UItemObject* ItemObject, UItemsContainer* OtherContainer)
{
	return IsItemObjectValid(ItemObject->GetItemId()) && IsValid(OtherContainer);
}

bool UInventoryComponent::HasAuthority() const
{
	return GetOwner()->HasAuthority();
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

UItemObject* UInventoryComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemSystemCore::GetItemObjectById(GetWorld(), ItemId);
}

bool UInventoryComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemSystemCore::IsItemObjectExist(GetWorld(), ItemId);
}
