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

void UPlayerInventoryManagerComponent::SetupInventoryManager(AController* InController, APlayerCharacter* InCharacter)
{
	CharacterRef = InCharacter;
	ControllerRef = InController;

	if (CharacterRef)
	{
		CharacterRef->OnLootContainer.AddUObject(this, &UPlayerInventoryManagerComponent::OnLootContainer);

		if (UInventoryComponent* CharInventoryComponent = CharacterRef->GetInventoryComponent())
		{
			PlayerInventory = CharInventoryComponent;
			
			if (UItemsContainer* CharContainer = CharInventoryComponent->GetItemsContainer())
			{
				PlayerItemsContainer = CharContainer;
				
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

void UPlayerInventoryManagerComponent::ResetInventoryManager()
{
	if (CharacterRef)
	{
		CharacterRef->OnLootContainer.RemoveAll(this);

		if (IsAuthority())
		{
			if (PlayerItemsContainer && Containers.Contains(PlayerItemsContainer))
			{
				Containers.Remove(PlayerItemsContainer);
			}
		}
	}
	
	CharacterRef = nullptr;
	ControllerRef = nullptr;
	PlayerInventory = nullptr;
	PlayerItemsContainer = nullptr;
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

void UPlayerInventoryManagerComponent::OnLootContainer(UInventoryComponent* InventoryComponent)
{
	if (UItemsContainer* Container = InventoryComponent->GetItemsContainer())
	{
		AddReplicatedContainer(Container);
	}
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

void UPlayerInventoryManagerComponent::ServerMoveItemToOtherContainer_Implementation(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	if (FromContainer)
	{
		FromContainer->MoveItemToOtherContainer(ItemObject, ToContainer);
	}
}

bool UPlayerInventoryManagerComponent::ServerMoveItemToOtherContainer_Validate(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId()) && IsValid(ToContainer);
}

bool UPlayerInventoryManagerComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UPlayerInventoryManagerComponent::IsAutonomousProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UPlayerInventoryManagerComponent::IsSimulatedProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_SimulatedProxy;
}

UItemObject* UPlayerInventoryManagerComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemSystemCore::GetItemObjectById(GetWorld(), ItemId);
}

bool UPlayerInventoryManagerComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemSystemCore::IsItemObjectExist(GetWorld(), ItemId);
}
