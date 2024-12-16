// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerComponent.h"
#include "ItemSystemCore.h"
#include "Character/PlayerCharacter.h"
#include "Components/EquipmentComponent.h"
#include "Components/EquipmentSlot.h"
#include "Components/InventoryComponent.h"
#include "Components/ItemsContainer.h"
#include "Engine/ActorChannel.h"
#include "Inventory/InventorySystemCore.h"
#include "Net/UnrealNetwork.h"

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, ReplicatedContainers,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryManagerComponent, ReplicatedEquipmentSlots,	COND_OwnerOnly);
}

bool UInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (UItemsContainer* Container : ReplicatedContainers)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(Container, *Bunch, *RepFlags);
		bReplicateSomething |= Container->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}

	for (UEquipmentSlot* EquipmentSlot : ReplicatedEquipmentSlots)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(EquipmentSlot, *Bunch, *RepFlags);
		bReplicateSomething |= EquipmentSlot->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
}

void UInventoryManagerComponent::SetupInventoryManager(AController* InController, APlayerCharacter* InCharacter)
{
	CharacterRef = InCharacter;
	ControllerRef = InController;

	if (!IsAuthority())
	{
		return;
	}
	
	if (CharacterRef)
	{
		CharacterRef->OnLootInventory.AddUObject(this, &UInventoryManagerComponent::OnLootInventory);

		if (UInventoryComponent* CharInventoryComponent = CharacterRef->GetInventoryComponent())
		{
			if (UItemsContainer* CharContainer = CharInventoryComponent->GetItemsContainer())
			{
				OwnItemsContainer = CharContainer;
				AddReplicatedContainer(CharContainer);
			}
		}

		if (UEquipmentComponent* CharEquipmentComponent = CharacterRef->GetEquipmentComponent())
		{
			TArray<UEquipmentSlot*> Slots = CharEquipmentComponent->GetEquipmentSlots();
			if (!Slots.IsEmpty())
			{
				OwnEquipmentSlots = Slots;
				
				for (UEquipmentSlot* Slot : Slots)
				{
					AddReplicatedEquipmentSlot(Slot);
				}
			}
		}
	}
}

void UInventoryManagerComponent::ResetInventoryManager()
{
	if (CharacterRef)
	{
		CharacterRef->OnLootInventory.RemoveAll(this);
	}
	
	CharacterRef = nullptr;
	ControllerRef = nullptr;
	
	if (!IsAuthority())
	{
		return;
	}
	
	RemoveReplicatedContainer(OwnItemsContainer);
	ReplicatedEquipmentSlots.Empty();
	OwnItemsContainer = nullptr;
	OwnEquipmentSlots.Empty();
}

void UInventoryManagerComponent::AddReplicatedContainer(UItemsContainer* Container)
{
	ReplicatedContainers.AddUnique(Container);
}

void UInventoryManagerComponent::RemoveReplicatedContainer(UItemsContainer* Container)
{
	if (ReplicatedContainers.Contains(Container))
	{
		ReplicatedContainers.Remove(Container);
	}
}

void UInventoryManagerComponent::AddReplicatedEquipmentSlot(UEquipmentSlot* EquipmentSlot)
{
	ReplicatedEquipmentSlots.AddUnique(EquipmentSlot);
}

void UInventoryManagerComponent::RemoveReplicatedEquipmentSlot(UEquipmentSlot* EquipmentSlot)
{
	if (ReplicatedEquipmentSlots.Contains(EquipmentSlot))
	{
		ReplicatedEquipmentSlots.Remove(EquipmentSlot);
	}
}

void UInventoryManagerComponent::OnLootInventory(UInventoryComponent* InventoryComponent)
{
	if (UItemsContainer* Container = InventoryComponent->GetItemsContainer())
	{
		AddReplicatedContainer(Container);
	}
}

void UInventoryManagerComponent::OnStopLootInventory(UInventoryComponent* InventoryComponent)
{
	if (UItemsContainer* Container = InventoryComponent->GetItemsContainer())
	{
		RemoveReplicatedContainer(Container);
	}
}

void UInventoryManagerComponent::ServerFindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::FindAvailablePlace(Container, ItemObject);
}

void UInventoryManagerComponent::ServerStackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem)
{
	UInventorySystemCore::StackItem(Container, SourceItem, TargetItem);
}

void UInventoryManagerComponent::ServerAddItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::AddItem(Container, ItemObject);
}

void UInventoryManagerComponent::ServerSplitItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::SplitItem(Container, ItemObject);
}

void UInventoryManagerComponent::ServerRemoveItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::RemoveItem(Container, ItemObject);
}

void UInventoryManagerComponent::ServerSubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject,
                                                            uint16 Amount)
{
	UInventorySystemCore::SubtractOrRemoveItem(Container, ItemObject, Amount);
}

void UInventoryManagerComponent::ServerMoveItemToOtherContainer(UItemsContainer* FromContainer,
                                                                UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	UInventorySystemCore::MoveItemToOtherContainer(FromContainer, ToContainer, ItemObject);
}

void UInventoryManagerComponent::ServerTryEquipItem(UItemObject* ItemObject)
{
	UInventorySystemCore::TryEquipItem(OwnEquipmentSlots, ItemObject, OwnItemsContainer);
}

void UInventoryManagerComponent::ServerEquipSlot(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject)
{
	UInventorySystemCore::EquipSlot(EquipmentSlot, ItemObject, OwnItemsContainer);
}

void UInventoryManagerComponent::ServerUnequipSlot(UEquipmentSlot* EquipmentSlot)
{
	UInventorySystemCore::UnequipSlot(EquipmentSlot);
}

void UInventoryManagerComponent::ServerMoveItemFromEquipmentSlot(UEquipmentSlot* EquipmentSlot)
{
	UInventorySystemCore::MoveItemFromEquipmentSlot(EquipmentSlot, OwnItemsContainer);
}

bool UInventoryManagerComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UInventoryManagerComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemSystemCore::IsItemObjectValid(GetWorld(), ItemId);
}

UItemObject* UInventoryManagerComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemSystemCore::GetItemObjectById(GetWorld(), ItemId);
}
