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

	DOREPLIFETIME_CONDITION(UCharacterInventoryComponent, EquipmentSlots,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCharacterInventoryComponent, FastUseSlots,		COND_OwnerOnly);
}

bool UCharacterInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                       FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UEquipmentSlot* EquipmentSlot : EquipmentSlots)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(EquipmentSlot, *Bunch, *RepFlags);
		bReplicateSomething |= EquipmentSlot->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	return bReplicateSomething;
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
			return Slot->CanEquipItem(ItemObject->GetDefinition());
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
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->IsEquipped())
		{
			ServerMoveItemFromSlot(SlotName);
		}
		
		UItemObject* EquippingItem = GetItemObjectById(ItemId);
		if (!EquippingItem)
		{
			return;
		}

		if (EquippingItem->GetAmount() > 1)
		{
			UItemObject* RemainedItem = UItemSystemCore::GenerateItemObject(GetWorld(), EquippingItem);
			if (!RemainedItem)
			{
				return;
			}

			RemainedItem->SetAmount(EquippingItem->GetAmount() - 1);
			ServerAddItem(RemainedItem);

			EquippingItem->SetAmount(1);
		}

		Slot->EquipSlot(EquippingItem);
	}
}

bool UCharacterInventoryComponent::ServerEquipSlot_Validate(const FString& SlotName, uint32 ItemId)
{
	return IsEquipmentSlotValid(SlotName) && IsItemObjectValid(ItemId);
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
			ServerAddItem(OldBoundObject);
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

		if (EquipmentSlot->CanEquipItem(BoundObject->GetDefinition()))
		{
			ServerEquipSlot(EquipmentSlot->GetSlotName(), BoundObject->GetItemId());
			ServerRemoveItem(BoundObject);
			break;
		}
	}
}

void UCharacterInventoryComponent::ServerEquipFastUseSlot_Implementation(uint8 SlotId)
{
	
}

bool UCharacterInventoryComponent::ServerEquipFastUseSlot_Validate(uint8 SlotId)
{
	return IsFastUseSlotValid(SlotId);
}

void UCharacterInventoryComponent::TryUseFastSlot(uint8 SlotId)
{
	
}

bool UCharacterInventoryComponent::IsEquipmentSlotValid(const FString& SlotName) const
{
	return IsValid(FindEquipmentSlot(SlotName));
}

bool UCharacterInventoryComponent::IsFastUseSlotValid(uint8 SlotId) const
{
	return true;
}
