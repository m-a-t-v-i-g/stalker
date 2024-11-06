// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "ItemObject.h"
#include "ItemsContainer.h"
#include "Engine/ActorChannel.h"
#include "Items/ItemsFunctionLibrary.h"
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

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                   FReplicationFlags* RepFlags)
{
	bool ReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UItemObject* EachItem : ItemsContainerRef->GetItems())
	{
		ReplicateSomething |= Channel->ReplicateSubobject(EachItem, *Bunch, *RepFlags);
	}
	return ReplicateSomething;
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

void UInventoryComponent::PreInitializeContainer()
{
	
}

void UInventoryComponent::PostInitializeContainer()
{
	
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
	return IsItemObjectValid(SourceItemId) && IsItemObjectValid(TargetItemId);
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

void UInventoryComponent::SplitItem(UItemObject* ItemObject)
{
	
}

void UInventoryComponent::RemoveItem(UItemObject* ItemObject)
{
	
}

void UInventoryComponent::SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount)
{
	if (ItemsContainerRef)
	{
		ItemsContainerRef->SubtractOrRemoveItem(ItemObject, Amount);
	}
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

void UInventoryComponent::DropItem(UItemObject* ItemObject)
{
}

void UInventoryComponent::UseItem(UItemObject* ItemObject)
{
}

bool UInventoryComponent::HasAuthority() const
{
	return GetOwner()->HasAuthority();
}

UItemObject* UInventoryComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemsFunctionLibrary::GetItemObjectById(GetWorld(), ItemId);
}

bool UInventoryComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemsFunctionLibrary::IsItemObjectExist(GetWorld(), ItemId);
}
