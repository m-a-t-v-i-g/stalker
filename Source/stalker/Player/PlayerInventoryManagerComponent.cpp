// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInventoryManagerComponent.h"

#include "ItemSystemCore.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Components/ItemsContainer.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UPlayerInventoryManagerComponent::UPlayerInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UPlayerInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPlayerInventoryManagerComponent, Containers, COND_OwnerOnly);
}

bool UPlayerInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (UItemsContainer* Container : Containers)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(Container, *Bunch, *RepFlags);
		bReplicateSomething |= Container->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
}

void UPlayerInventoryManagerComponent::SetupInventoryManager(APlayerCharacter* InCharacter)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		Containers.AddUnique(InCharacter->GetInventoryComponent()->GetItemsContainer());
		
		InCharacter->OnContainerInteraction.AddUObject(this, &UPlayerInventoryManagerComponent::OnContainerInteract);
	}
}

void UPlayerInventoryManagerComponent::InitCharacterInfo(AController* InController)
{
	
}

void UPlayerInventoryManagerComponent::AddReplicatedContainer(UItemsContainer* Container)
{
	if (!Containers.Contains(Container))
	{
		Containers.Add(Container);
	}
}

void UPlayerInventoryManagerComponent::RemoveReplicatedContainer(UItemsContainer* Container)
{
	if (Containers.Contains(Container))
	{
		Containers.Remove(Container);
	}
}

void UPlayerInventoryManagerComponent::OnContainerInteract(UInventoryComponent* InventoryComponent)
{
	AddReplicatedContainer(InventoryComponent->GetItemsContainer());
}

void UPlayerInventoryManagerComponent::ServerFindAvailablePlace_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->FindAvailablePlace(ItemObject);
	}
}

bool UPlayerInventoryManagerComponent::ServerFindAvailablePlace_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UPlayerInventoryManagerComponent::ServerStackItem_Implementation(UItemsContainer* Container,
                                                                      UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (Container)
	{
		Container->StackItem(SourceItem, TargetItem);
	}
}

bool UPlayerInventoryManagerComponent::ServerStackItem_Validate(UItemsContainer* Container, UItemObject* SourceItem,
                                                                UItemObject* TargetItem)
{
	return IsItemObjectValid(SourceItem->GetItemId());
}

void UPlayerInventoryManagerComponent::ServerAddItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->AddItem(ItemObject);
	}
}

bool UPlayerInventoryManagerComponent::ServerAddItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UPlayerInventoryManagerComponent::ServerSplitItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->SplitItem(ItemObject);
	}
}

bool UPlayerInventoryManagerComponent::ServerSplitItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UPlayerInventoryManagerComponent::ServerRemoveItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->RemoveItem(ItemObject);
	}
}

bool UPlayerInventoryManagerComponent::ServerRemoveItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UPlayerInventoryManagerComponent::ServerSubtractOrRemoveItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount)
{
	if (Container)
	{
		Container->SubtractOrRemoveItem(ItemObject, Amount);
	}
}

bool UPlayerInventoryManagerComponent::ServerSubtractOrRemoveItem_Validate(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UPlayerInventoryManagerComponent::ServerMoveItemToOtherContainer_Implementation(UItemsContainer* Container, UItemObject* ItemObject, UItemsContainer* OtherContainer)
{
	if (Container)
	{
		Container->MoveItemToOtherContainer(ItemObject, OtherContainer);
	}
}

bool UPlayerInventoryManagerComponent::ServerMoveItemToOtherContainer_Validate(UItemsContainer* Container, UItemObject* ItemObject, UItemsContainer* OtherContainer)
{
	return IsItemObjectValid(ItemObject->GetItemId()) && IsValid(OtherContainer);
}

UItemObject* UPlayerInventoryManagerComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemSystemCore::GetItemObjectById(GetWorld(), ItemId);
}

bool UPlayerInventoryManagerComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemSystemCore::IsItemObjectExist(GetWorld(), ItemId);
}
