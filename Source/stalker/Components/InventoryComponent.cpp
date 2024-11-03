// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "ItemObject.h"
#include "ItemsContainer.h"
#include "Engine/ActorChannel.h"
#include "Game/StalkerGameState.h"
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

void UInventoryComponent::StackItem(UItemObject* SourceObject, UItemObject* TargetItem)
{
	
}

void UInventoryComponent::ServerAddItem_Implementation(uint32 ItemId)
{
	if (!IsItemObjectExist(ItemId))
	{
		return;
	}
	
	if (ItemsContainerRef)
	{
		ItemsContainerRef->AddItem(GetItemObjectById(ItemId));
	}
}

bool UInventoryComponent::ServerAddItem_Validate(uint32 ItemId)
{
	return IsItemObjectExist(ItemId);
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
	if (auto GameState = Cast<AStalkerGameState>(GetWorld()->GetGameState()))
	{
		return GameState->GetItemObjectById(ItemId);
	}
	return nullptr;
}

bool UInventoryComponent::IsItemObjectExist(uint32 ItemId) const
{
	if (auto GameState = Cast<AStalkerGameState>(GetWorld()->GetGameState()))
	{
		return GameState->IsItemObjectExist(ItemId);
	}
	return false;
}
