// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterInventoryComponent.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "Components/EquipmentSlot.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCharacterInventoryComponent, EquipmentSlots, COND_OwnerOnly);
}

bool UCharacterInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                       FReplicationFlags* RepFlags)
{
	bool ReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UEquipmentSlot* EachSlot : EquipmentSlots)
	{
		ReplicateSomething |= Channel->ReplicateSubobject(EachSlot, *Bunch, *RepFlags);
	}
	return ReplicateSomething;
}

void UCharacterInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		for (UEquipmentSlot* Slot : EquipmentSlots)
		{
			Slot->AddStartingData();
		}
	}
}

void UCharacterInventoryComponent::EquipSlot(const FString& SlotName, uint32 ItemId)
{
	ServerEquipSlot(SlotName, ItemId);
}

void UCharacterInventoryComponent::UnequipSlot(const FString& SlotName)
{
	ServerUnequipSlot(SlotName);
}

bool UCharacterInventoryComponent::CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (UEquipmentSlot* Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->GetBoundObject() != ItemObject)
		{
			return Slot->CanEquipItem(ItemObject->ItemDefinition);
		}
	}
	return false;
}

UEquipmentSlot* UCharacterInventoryComponent::FindEquipmentSlot(const FString& SlotName) const
{
	if (EquipmentSlots.Num() > 0)
	{
		auto Predicate = EquipmentSlots.FindByPredicate([&, SlotName] (const UEquipmentSlot* Slot)
		{
			return Slot->GetSlotName() == SlotName;
		});

		if (Predicate)
		{
			return *Predicate;
		}
	}
	return nullptr;
}

void UCharacterInventoryComponent::ServerEquipSlot_Implementation(const FString& SlotName, uint32 ItemId)
{
	UItemObject* ItemObject = GetItemObjectById(ItemId);
	if (!ItemObject)
	{
		return;
	}
	
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->IsEquipped())
		{
			ServerMoveItemFromSlot(SlotName);
		}

		UItemObject* OtherItem = ItemObject;

		if (ItemObject->GetItemInstance()->Amount > 1)
		{
			OtherItem = UItemSystemCore::GenerateItemObject(GetWorld(), ItemObject);
			OtherItem->SetAmount(1);
		}

		ServerSubtractOrRemoveItem(ItemObject->GetItemId(), 1);
		Slot->EquipSlot(OtherItem);
	}
}

bool UCharacterInventoryComponent::ServerEquipSlot_Validate(const FString& SlotName, uint32 ItemId)
{
	return IsItemObjectValid(ItemId);
}

void UCharacterInventoryComponent::ServerUnequipSlot_Implementation(const FString& SlotName)
{
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->IsEquipped())
		{
			Slot->UnequipSlot();
		}
	}
}

bool UCharacterInventoryComponent::ServerUnequipSlot_Validate(const FString& SlotName)
{
	return IsEquipmentSlotValid(SlotName);
}

void UCharacterInventoryComponent::ServerMoveItemFromSlot_Implementation(const FString& SlotName)
{
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (UItemObject* OldBoundObject = Slot->GetBoundObject())
		{
			Slot->UnequipSlot();
			ServerAddItem(OldBoundObject->GetItemId());
		}
	}
}

bool UCharacterInventoryComponent::ServerMoveItemFromSlot_Validate(const FString& SlotName)
{
	return IsEquipmentSlotValid(SlotName);
}

void UCharacterInventoryComponent::TryEquipItem(UItemObject* BoundObject)
{
	for (auto EquipmentSlot : EquipmentSlots)
	{
		if (!IsValid(EquipmentSlot))
		{
			continue;
		}

		if (EquipmentSlot->CanEquipItem(BoundObject->ItemDefinition))
		{
			EquipSlot(EquipmentSlot->GetSlotName(), BoundObject->GetItemId());
			break;
		}
	}
}

bool UCharacterInventoryComponent::IsEquipmentSlotValid(const FString& SlotName) const
{
	return IsValid(FindEquipmentSlot(SlotName));
}
