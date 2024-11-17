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
	
	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryComponent, ItemsContainerRef, COND_OwnerOnly);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	bReplicateSomething |= Channel->ReplicateSubobject(ItemsContainerRef, *Bunch, *RepFlags);
	bReplicateSomething |= ItemsContainerRef->ReplicateSubobjects(Channel, Bunch, RepFlags);
	return bReplicateSomething;
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

void UInventoryComponent::ServerFindAvailablePlace_Implementation(uint32 ItemId)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->FindAvailablePlace(GetItemObjectById(ItemId));
	}
}

bool UInventoryComponent::ServerFindAvailablePlace_Validate(uint32 ItemId)
{
	return IsItemObjectValid(ItemId);
}

void UInventoryComponent::ServerStackItem_Implementation(uint32 SourceItemId, uint32 TargetItemId)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->StackItem(GetItemObjectById(SourceItemId), GetItemObjectById(TargetItemId));
	}
}

bool UInventoryComponent::ServerStackItem_Validate(uint32 SourceItemId, uint32 TargetItemId)
{
	return IsItemObjectValid(SourceItemId);
}

void UInventoryComponent::ServerAddItem_Implementation(uint32 ItemId)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->AddItem(GetItemObjectById(ItemId));
	}
}

bool UInventoryComponent::ServerAddItem_Validate(uint32 ItemId)
{
	return IsItemObjectValid(ItemId);
}

void UInventoryComponent::ServerSplitItem_Implementation(uint32 ItemId)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->SplitItem(GetItemObjectById(ItemId));
	}
}

bool UInventoryComponent::ServerSplitItem_Validate(uint32 ItemId)
{
	return IsItemObjectValid(ItemId);
}

void UInventoryComponent::ServerRemoveItem_Implementation(uint32 ItemId)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->RemoveItem(GetItemObjectById(ItemId));
	}
}

bool UInventoryComponent::ServerRemoveItem_Validate(uint32 ItemId)
{
	return IsItemObjectValid(ItemId);
}

void UInventoryComponent::ServerSubtractOrRemoveItem_Implementation(uint32 ItemId, uint16 Amount)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->SubtractOrRemoveItem(GetItemObjectById(ItemId), Amount);
	}
}

bool UInventoryComponent::ServerSubtractOrRemoveItem_Validate(uint32 ItemId, uint16 Amount)
{
	return IsItemObjectValid(ItemId);
}

void UInventoryComponent::ServerMoveItemToOtherContainer_Implementation(uint32 ItemId, UItemsContainer* OtherContainer)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->MoveItemToOtherContainer(GetItemObjectById(ItemId), OtherContainer);
	}
}

bool UInventoryComponent::ServerMoveItemToOtherContainer_Validate(uint32 ItemId, UItemsContainer* OtherContainer)
{
	return IsItemObjectValid(ItemId) && IsValid(OtherContainer);
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
