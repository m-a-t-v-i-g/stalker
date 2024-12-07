// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerComponent.h"

#include "ItemSystemCore.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Components/ItemsContainer.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, Containers, COND_OwnerOnly);
}

bool UInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (UItemsContainer* Container : Containers)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(Container, *Bunch, *RepFlags);
		bReplicateSomething |= Container->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
}

void UInventoryManagerComponent::SetupInventoryManager(AController* InController, APlayerCharacter* InCharacter)
{
	CharacterRef = InCharacter;
	ControllerRef = InController;

	if (CharacterRef)
	{
		CharacterRef->OnLootContainer.AddUObject(this, &UInventoryManagerComponent::OnLootContainer);

		if (UInventoryComponent* CharInventoryComponent = CharacterRef->GetInventoryComponent())
		{
			OwnInventory = CharInventoryComponent;
			
			if (UItemsContainer* CharContainer = CharInventoryComponent->GetItemsContainer())
			{
				OwnItemsContainer = CharContainer;
				
				if (IsAuthority())
				{
					if (!Containers.Contains(CharContainer))
					{
						Containers.AddUnique(CharContainer);
					}
				}
			}
		}
	}
}

void UInventoryManagerComponent::ResetInventoryManager()
{
	if (CharacterRef)
	{
		CharacterRef->OnLootContainer.RemoveAll(this);

		if (IsAuthority())
		{
			if (OwnItemsContainer && Containers.Contains(OwnItemsContainer))
			{
				Containers.Remove(OwnItemsContainer);
			}
		}
	}
	
	CharacterRef = nullptr;
	ControllerRef = nullptr;
	OwnInventory = nullptr;
	OwnItemsContainer = nullptr;
}

void UInventoryManagerComponent::AddReplicatedContainer(UItemsContainer* Container)
{
	if (!Containers.Contains(Container))
	{
		Containers.Add(Container);
	}
}

void UInventoryManagerComponent::RemoveReplicatedContainer(UItemsContainer* Container)
{
	if (Containers.Contains(Container))
	{
		Containers.Remove(Container);
	}
}

void UInventoryManagerComponent::OnLootContainer(UInventoryComponent* InventoryComponent)
{
	if (UItemsContainer* Container = InventoryComponent->GetItemsContainer())
	{
		AddReplicatedContainer(Container);
	}
}

void UInventoryManagerComponent::OnStopLootContainer(UInventoryComponent* InventoryComponent)
{
	if (UItemsContainer* Container = InventoryComponent->GetItemsContainer())
	{
		RemoveReplicatedContainer(Container);
	}
}

void UInventoryManagerComponent::ServerFindAvailablePlace_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->FindAvailablePlace(ItemObject);
	}
}

bool UInventoryManagerComponent::ServerFindAvailablePlace_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerStackItem_Implementation(UItemsContainer* Container,
                                                                      UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (Container)
	{
		Container->StackItem(SourceItem, TargetItem);
	}
}

bool UInventoryManagerComponent::ServerStackItem_Validate(UItemsContainer* Container, UItemObject* SourceItem,
                                                                UItemObject* TargetItem)
{
	return IsItemObjectValid(SourceItem->GetItemId());
}

void UInventoryManagerComponent::ServerAddItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->AddItem(ItemObject);
	}
}

bool UInventoryManagerComponent::ServerAddItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerSplitItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->SplitItem(ItemObject);
	}
}

bool UInventoryManagerComponent::ServerSplitItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerRemoveItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (Container)
	{
		Container->RemoveItem(ItemObject);
	}
}

bool UInventoryManagerComponent::ServerRemoveItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerSubtractOrRemoveItem_Implementation(
	UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount)
{
	if (Container)
	{
		Container->SubtractOrRemoveItem(ItemObject, Amount);
	}
}

bool UInventoryManagerComponent::ServerSubtractOrRemoveItem_Validate(UItemsContainer* Container,
                                                                     UItemObject* ItemObject, uint16 Amount)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerMoveItemToOtherContainer_Implementation(
	UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	if (FromContainer)
	{
		FromContainer->MoveItemToOtherContainer(ItemObject, ToContainer);
	}
}

bool UInventoryManagerComponent::ServerMoveItemToOtherContainer_Validate(
	UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId()) && IsValid(ToContainer);
}

bool UInventoryManagerComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UInventoryManagerComponent::IsAutonomousProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UInventoryManagerComponent::IsSimulatedProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_SimulatedProxy;
}

UItemObject* UInventoryManagerComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemSystemCore::GetItemObjectById(GetWorld(), ItemId);
}

bool UInventoryManagerComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemSystemCore::IsItemObjectExist(GetWorld(), ItemId);
}
