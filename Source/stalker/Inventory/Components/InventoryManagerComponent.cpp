// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerComponent.h"
#include "ItemSystemCore.h"
#include "PawnInteractionComponent.h"
#include "Components/EquipmentComponent.h"
#include "Components/EquipmentSlot.h"
#include "Components/InventoryComponent.h"
#include "Components/ItemsContainer.h"
#include "Containers/ContainerActor.h"
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

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedContainers,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedEquipmentSlots,	COND_OwnerOnly);
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

void UInventoryManagerComponent::SetupInventoryManager(APawn* InPawn, AController* InController)
{
	PawnRef = InPawn;
	ControllerRef = InController;

	if (!IsAuthority())
	{
		return;
	}
	
	if (PawnRef)
	{
		if (auto InventoryComponent = PawnRef->GetComponentByClass<UInventoryComponent>())
		{
			if (UItemsContainer* ItemsContainer = InventoryComponent->GetItemsContainer())
			{
				OwnItemsContainer = ItemsContainer;
				AddReplicatedContainer(ItemsContainer);
			}
		}

		if (auto EquipmentComponent = PawnRef->GetComponentByClass<UEquipmentComponent>())
		{
			TArray<UEquipmentSlot*> Slots = EquipmentComponent->GetEquipmentSlots();
			if (!Slots.IsEmpty())
			{
				OwnEquipmentSlots = Slots;
				
				for (UEquipmentSlot* Slot : Slots)
				{
					AddReplicatedEquipmentSlot(Slot);
				}
			}
		}

		if (auto InteractionComponent = PawnRef->GetComponentByClass<UPawnInteractionComponent>())
		{
			InteractionComponent->OnPossibleInteractionAdd.AddUObject(this, &UInventoryManagerComponent::OnPossibleInteractionAdd);
			InteractionComponent->OnPossibleInteractionRemove.AddUObject(this, &UInventoryManagerComponent::OnPossibleInteractionRemove);
		}
	}
}

void UInventoryManagerComponent::ResetInventoryManager()
{
	if (PawnRef)
	{
		if (auto CharInteractionComponent = PawnRef->GetComponentByClass<UPawnInteractionComponent>())
		{
			CharInteractionComponent->OnPossibleInteractionAdd.RemoveAll(this);
			CharInteractionComponent->OnPossibleInteractionRemove.RemoveAll(this);
		}
	}
	
	PawnRef = nullptr;
	ControllerRef = nullptr;
	
	if (!IsAuthority())
	{
		return;
	}
	
	ReplicatedContainers.Empty();
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

void UInventoryManagerComponent::ServerFindAvailablePlace_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::FindAvailablePlace(Container, ItemObject);
}

bool UInventoryManagerComponent::ServerFindAvailablePlace_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerStackItem_Implementation(UItemsContainer* Container, UItemObject* SourceItem,
                                                                UItemObject* TargetItem)
{
	UInventorySystemCore::StackItem(Container, SourceItem, TargetItem);
}

bool UInventoryManagerComponent::ServerStackItem_Validate(UItemsContainer* Container, UItemObject* SourceItem,
                                                          UItemObject* TargetItem)
{
	return IsItemObjectValid(SourceItem->GetItemId()) && IsItemObjectValid(TargetItem->GetItemId());
}

void UInventoryManagerComponent::ServerAddItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::AddItem(Container, ItemObject);
}

bool UInventoryManagerComponent::ServerAddItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerSplitItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::SplitItem(Container, ItemObject);
}

bool UInventoryManagerComponent::ServerSplitItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerRemoveItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::RemoveItem(Container, ItemObject);
}

bool UInventoryManagerComponent::ServerRemoveItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerSubtractOrRemoveItem_Implementation(
	UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount)
{
	UInventorySystemCore::SubtractOrRemoveItem(Container, ItemObject, Amount);
}

bool UInventoryManagerComponent::ServerSubtractOrRemoveItem_Validate(UItemsContainer* Container,
                                                                     UItemObject* ItemObject, uint16 Amount)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerMoveItemToOtherContainer_Implementation(
	UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	UInventorySystemCore::MoveItemToOtherContainer(FromContainer, ToContainer, ItemObject);
}

bool UInventoryManagerComponent::ServerMoveItemToOtherContainer_Validate(
	UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerTryEquipItem_Implementation(UItemObject* ItemObject)
{
	UInventorySystemCore::TryEquipItem(OwnEquipmentSlots, ItemObject, OwnItemsContainer);
}

bool UInventoryManagerComponent::ServerTryEquipItem_Validate(UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerEquipSlot_Implementation(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject)
{
	UInventorySystemCore::EquipSlot(EquipmentSlot, ItemObject, OwnItemsContainer);
}

bool UInventoryManagerComponent::ServerEquipSlot_Validate(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject)
{
	return IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::ServerUnequipSlot_Implementation(UEquipmentSlot* EquipmentSlot)
{
	UInventorySystemCore::UnequipSlot(EquipmentSlot);
}

bool UInventoryManagerComponent::ServerUnequipSlot_Validate(UEquipmentSlot* EquipmentSlot)
{
	return IsValid(EquipmentSlot);
}

void UInventoryManagerComponent::ServerMoveItemFromEquipmentSlot_Implementation(UEquipmentSlot* EquipmentSlot)
{
	UInventorySystemCore::MoveItemFromEquipmentSlot(EquipmentSlot, OwnItemsContainer);
}

bool UInventoryManagerComponent::ServerMoveItemFromEquipmentSlot_Validate(UEquipmentSlot* EquipmentSlot)
{
	return IsValid(EquipmentSlot);
}

bool UInventoryManagerComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

void UInventoryManagerComponent::OnPossibleInteractionAdd(AActor* TargetActor)
{
	if (auto Container = Cast<AContainerActor>(TargetActor))
	{
		UInventoryComponent* InventoryComponent = Container->GetInventoryComponent();
		if (!InventoryComponent)
		{
			return;
		}
		
		if (UItemsContainer* ItemsContainer = InventoryComponent->GetItemsContainer())
		{
			AddReplicatedContainer(ItemsContainer);
		}
	}
}

void UInventoryManagerComponent::OnPossibleInteractionRemove(AActor* TargetActor)
{
	if (auto Container = Cast<AContainerActor>(TargetActor))
	{
		UInventoryComponent* InventoryComponent = Container->GetInventoryComponent();
		if (!InventoryComponent)
		{
			return;
		}
		
		if (UItemsContainer* ItemsContainer = InventoryComponent->GetItemsContainer())
		{
			RemoveReplicatedContainer(ItemsContainer);
		}
	}
}

bool UInventoryManagerComponent::IsItemObjectValid(uint32 ItemId) const
{
	return UItemSystemCore::IsItemObjectValid(GetWorld(), ItemId);
}

UItemObject* UInventoryManagerComponent::GetItemObjectById(uint32 ItemId) const
{
	return UItemSystemCore::GetItemObjectById(GetWorld(), ItemId);
}
