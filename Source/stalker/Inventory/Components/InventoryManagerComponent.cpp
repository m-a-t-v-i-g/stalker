// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryManagerComponent.h"
#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "InventoryComponent.h"
#include "InventorySystemCore.h"
#include "ItemsContainer.h"
#include "ItemSystemCore.h"
#include "PawnInteractionComponent.h"
#include "Containers/ContainerActor.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UInventoryManagerComponent::UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

	if (PawnRef)
	{
		if (auto InventoryComponent = PawnRef->GetComponentByClass<UInventoryComponent>())
		{
			if (UItemsContainer* ItemsContainer = InventoryComponent->GetItemsContainer())
			{
				OwnItemsContainer = ItemsContainer;

				if (HasAuthority())
				{
					AddReplicatedContainer(ItemsContainer);
				}
			}
		}

		if (auto EquipmentComponent = PawnRef->GetComponentByClass<UEquipmentComponent>())
		{
			TArray<UEquipmentSlot*> Slots = EquipmentComponent->GetEquipmentSlots();
			if (!Slots.IsEmpty())
			{
				OwnEquipmentSlots = Slots;

				if (HasAuthority())
				{
					for (UEquipmentSlot* Slot : Slots)
					{
						AddReplicatedEquipmentSlot(Slot);
					}
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
	
	if (!HasAuthority())
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

void UInventoryManagerComponent::StackItem(UItemsContainer* Container, UObject* Source, UItemObject* SourceItem, UItemObject* TargetItem)
{
	check(Container);
	check(Source);
	check(SourceItem);
	check(TargetItem);
	
	if (!ControllerRef)
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Container_StackItem(Container, SourceItem, TargetItem);
		RemoveItemFromSource(Source, SourceItem);
	}

	ServerStackItem(Container, Source, SourceItem, TargetItem);
}

void UInventoryManagerComponent::ServerStackItem_Implementation(UItemsContainer* Container, UObject* Source, UItemObject* SourceItem,
                                                                UItemObject* TargetItem)
{
	UInventorySystemCore::Container_StackItem(Container, SourceItem, TargetItem);
	RemoveItemFromSource(Source, SourceItem);
}

bool UInventoryManagerComponent::ServerStackItem_Validate(UItemsContainer* Container, UObject* Source, UItemObject* SourceItem,
                                                          UItemObject* TargetItem)
{
	return IsValid(Container) && IsValid(Source) && IsItemObjectValid(SourceItem->GetItemId()) && IsItemObjectValid(
		TargetItem->GetItemId());
}

void UInventoryManagerComponent::SplitItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	check(Container);
	check(ItemObject);

	if (!ControllerRef)
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Container_SplitItem(Container, ItemObject);
	}

	ServerSplitItem(Container, ItemObject);
}

void UInventoryManagerComponent::ServerSplitItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::Container_SplitItem(Container, ItemObject);
}

bool UInventoryManagerComponent::ServerSplitItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsValid(Container) && IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::AddItem(UItemsContainer* Container, UObject* Source, UItemObject* ItemObject)
{
	check(Container);
	check(Source);
	check(ItemObject);

	if (!ControllerRef)
	{
		return;
	}
	
	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Container_AddItem(Container, ItemObject);
		RemoveItemFromSource(Source, ItemObject);
	}

	ServerAddItem(Container, Source, ItemObject);
}

void UInventoryManagerComponent::ServerAddItem_Implementation(UItemsContainer* Container, UObject* Source, UItemObject* ItemObject)
{
	UInventorySystemCore::Container_AddItem(Container, ItemObject);
	RemoveItemFromSource(Source, ItemObject);
}

bool UInventoryManagerComponent::ServerAddItem_Validate(UItemsContainer* Container, UObject* Source, UItemObject* ItemObject)
{
	return IsValid(Container) && IsValid(Source) && IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::RemoveItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	check(Container);
	check(ItemObject);

	if (!ControllerRef)
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Container_RemoveItem(Container, ItemObject);
	}

	ServerRemoveItem(Container, ItemObject);
}

void UInventoryManagerComponent::ServerRemoveItem_Implementation(UItemsContainer* Container, UItemObject* ItemObject)
{
	UInventorySystemCore::Container_RemoveItem(Container, ItemObject);
}

bool UInventoryManagerComponent::ServerRemoveItem_Validate(UItemsContainer* Container, UItemObject* ItemObject)
{
	return IsValid(Container) && IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::MoveItemToOtherContainer(UItemsContainer* FromContainer, UItemsContainer* ToContainer,
														  UItemObject* ItemObject)
{
	check(FromContainer);
	check(ToContainer);
	check(ItemObject);

	if (!ControllerRef)
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Container_MoveItemToOtherContainer(FromContainer, ToContainer, ItemObject);
	}
	
	ServerMoveItemToOtherContainer(FromContainer, ToContainer, ItemObject);
}

void UInventoryManagerComponent::ServerMoveItemToOtherContainer_Implementation(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	UInventorySystemCore::Container_MoveItemToOtherContainer(FromContainer, ToContainer, ItemObject);
}

bool UInventoryManagerComponent::ServerMoveItemToOtherContainer_Validate(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject)
{
	return IsValid(FromContainer) && IsValid(ToContainer) && IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::TryEquipItem(UObject* Source, UItemObject* ItemObject)
{
	check(ItemObject);

	if (!ControllerRef)
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		for (UEquipmentSlot* EquipmentSlot : OwnEquipmentSlots)
		{
			if (EquipmentSlot->CanEquipItem(ItemObject->GetDefinition()))
			{
				RemoveItemFromSource(Source, ItemObject);
			
				if (EquipmentSlot->IsEquipped())
				{
					UInventorySystemCore::Slot_MoveItemToContainer(EquipmentSlot, OwnItemsContainer);
				}

				UInventorySystemCore::Slot_EquipItem(EquipmentSlot, ItemObject);
				break;
			}
		}
	}
	
	ServerTryEquipItem(Source, ItemObject);
}

void UInventoryManagerComponent::ServerTryEquipItem_Implementation(UObject* Source, UItemObject* ItemObject)
{
	check(ItemObject);
	
	for (UEquipmentSlot* EquipmentSlot : OwnEquipmentSlots)
	{
		if (!EquipmentSlot->CanEquipItem(ItemObject->GetDefinition()))
		{
			continue;
		}

		RemoveItemFromSource(Source, ItemObject);

		if (EquipmentSlot->IsEquipped())
		{
			UInventorySystemCore::Slot_MoveItemToContainer(EquipmentSlot, OwnItemsContainer);
		}

		if (ItemObject->GetAmount() > 1)
		{
			UItemObject* RemainedItem = UItemSystemCore::GenerateItemObject(OwnItemsContainer->GetWorld(), ItemObject);
			if (!RemainedItem)
			{
				return;
			}

			RemainedItem->SetAmount(ItemObject->GetAmount() - 1);
			UInventorySystemCore::Container_AddItem(OwnItemsContainer, RemainedItem);
			
			ItemObject->SetAmount(1);
		}

		UInventorySystemCore::Slot_EquipItem(EquipmentSlot, ItemObject);
		break;
	}
}

bool UInventoryManagerComponent::ServerTryEquipItem_Validate(UObject* Source, UItemObject* ItemObject)
{
	return IsValid(Source) && IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::EquipSlot(UEquipmentSlot* EquipmentSlot, UObject* Source, UItemObject* ItemObject)
{
	check(EquipmentSlot);
	check(Source);
	check(ItemObject);

	if (!ControllerRef)
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		if (EquipmentSlot->CanEquipItem(ItemObject->GetDefinition()))
		{
			RemoveItemFromSource(Source, ItemObject);

			if (EquipmentSlot->IsEquipped())
			{
				UInventorySystemCore::Slot_MoveItemToContainer(EquipmentSlot, OwnItemsContainer);
			}

			UInventorySystemCore::Slot_EquipItem(EquipmentSlot, ItemObject);
		}
	}

	ServerEquipSlot(EquipmentSlot, Source, ItemObject);
}

void UInventoryManagerComponent::ServerEquipSlot_Implementation(UEquipmentSlot* EquipmentSlot, UObject* Source, UItemObject* ItemObject)
{
	if (EquipmentSlot->CanEquipItem(ItemObject->GetDefinition()))
	{
		RemoveItemFromSource(Source, ItemObject);

		if (EquipmentSlot->IsEquipped())
		{
			UInventorySystemCore::Slot_MoveItemToContainer(EquipmentSlot, OwnItemsContainer);
		}

		if (ItemObject->GetAmount() > 1)
		{
			UItemObject* RemainedItem = UItemSystemCore::GenerateItemObject(OwnItemsContainer->GetWorld(), ItemObject);
			if (!RemainedItem)
			{
				return;
			}

			RemainedItem->SetAmount(ItemObject->GetAmount() - 1);
			UInventorySystemCore::Container_AddItem(OwnItemsContainer, RemainedItem);

			ItemObject->SetAmount(1);
		}

		UInventorySystemCore::Slot_EquipItem(EquipmentSlot, ItemObject);
	}
}

bool UInventoryManagerComponent::ServerEquipSlot_Validate(UEquipmentSlot* EquipmentSlot, UObject* Source, UItemObject* ItemObject)
{
	return IsValid(EquipmentSlot) && IsItemObjectValid(ItemObject->GetItemId());
}

void UInventoryManagerComponent::UnequipSlot(UEquipmentSlot* EquipmentSlot)
{
	check(EquipmentSlot);

	if (!ControllerRef)
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Slot_UnequipItem(EquipmentSlot);
	}

	ServerUnequipSlot(EquipmentSlot);
}

void UInventoryManagerComponent::ServerUnequipSlot_Implementation(UEquipmentSlot* EquipmentSlot)
{
	UInventorySystemCore::Slot_UnequipItem(EquipmentSlot);
}

bool UInventoryManagerComponent::ServerUnequipSlot_Validate(UEquipmentSlot* EquipmentSlot)
{
	return IsValid(EquipmentSlot);
}

void UInventoryManagerComponent::MoveItemFromEquipmentSlotToContainer(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container)
{
	check(EquipmentSlot);

	if (!ControllerRef)
	{
		return;
	}

	if (!HasAuthority() && IsLocalController())
	{
		UInventorySystemCore::Slot_MoveItemToContainer(EquipmentSlot, Container);
	}

	ServerMoveItemFromEquipmentSlotToContainer(EquipmentSlot, Container);
}

void UInventoryManagerComponent::ServerMoveItemFromEquipmentSlotToContainer_Implementation(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container)
{
	UInventorySystemCore::Slot_MoveItemToContainer(EquipmentSlot, Container);
}

bool UInventoryManagerComponent::ServerMoveItemFromEquipmentSlotToContainer_Validate(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container)
{
	return IsValid(EquipmentSlot);
}

void UInventoryManagerComponent::RemoveItemFromSource(UObject* Source, UItemObject* ItemObject)
{
	if (auto SourceContainer = Cast<UItemsContainer>(Source))
	{
		if (SourceContainer->Contains(ItemObject))
		{
			UInventorySystemCore::Container_RemoveItem(SourceContainer, ItemObject);
		}
		return;
	}

	if (auto SourceSlot = Cast<UEquipmentSlot>(Source))
	{
		if (SourceSlot->CanEquipItem(ItemObject->GetDefinition()))
		{
			UInventorySystemCore::Slot_UnequipItem(SourceSlot);
		}
		return;
	}
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
			if (HasAuthority())
			{
				AddReplicatedContainer(ItemsContainer);
			}
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
			if (HasAuthority())
			{
				RemoveReplicatedContainer(ItemsContainer);
			}
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
