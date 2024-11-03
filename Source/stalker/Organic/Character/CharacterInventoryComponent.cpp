// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterInventoryComponent.h"
#include "ItemObject.h"
#include "Components/EquipmentSlot.h"
#include "Engine/ActorChannel.h"
#include "Items/ItemsFunctionLibrary.h"
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

void UCharacterInventoryComponent::EquipSlot(const FString& SlotName, UItemObject* ItemObject)
{
	ServerEquipSlot(SlotName, ItemObject);
}

void UCharacterInventoryComponent::UnequipAndEquipSlot(const FString& SlotName, UItemObject* ItemObject)
{
	ServerUnequipAndEquipSlot(SlotName, ItemObject);
}

void UCharacterInventoryComponent::UnequipSlot(const FString& SlotName)
{
	ServerUnequipSlot(SlotName);
}

bool UCharacterInventoryComponent::CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		return Slot->CanEquipItem(ItemObject);
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

void UCharacterInventoryComponent::ServerEquipSlot_Implementation(const FString& SlotName, UItemObject* ItemObject)
{
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->IsEquipped())
		{
			UnequipSlot(SlotName);
		}
		
		UItemObject* OtherItem = ItemObject;
		uint16 ItemAmount = OtherItem->GetItemInstance()->Amount;
		
		if (ItemAmount > 1)
		{
			OtherItem = UItemsFunctionLibrary::GenerateItemObject(GetWorld(), ItemObject);
			OtherItem->SetAmount(1);
		}

		SubtractOrRemoveItem(ItemObject, 1);
		Slot->EquipSlot(OtherItem);
	}
}

void UCharacterInventoryComponent::ServerUnequipAndEquipSlot_Implementation(const FString& SlotName, UItemObject* ItemObject)
{
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->IsEquipped())
		{
			auto OldBoundObject = Slot->GetBoundObject();
			check(OldBoundObject);

			UnequipSlot(SlotName);
			ServerAddItem(OldBoundObject->GetItemId());
			EquipSlot(SlotName, ItemObject);
		}
	}
}

void UCharacterInventoryComponent::ServerUnequipSlot_Implementation(const FString& SlotName)
{
	if (auto Slot = FindEquipmentSlot(SlotName))
	{
		if (Slot->IsEquipped())
		{
			auto OldBoundObject = Slot->GetBoundObject();
			check(OldBoundObject);

			Slot->UnEquipSlot();
		}
	}
}

void UCharacterInventoryComponent::TryEquipItem(UItemObject* BoundObject)
{
	for (auto EquipmentSlot : EquipmentSlots)
	{
		if (!IsValid(EquipmentSlot))
		{
			continue;
		}

		if (EquipmentSlot->CanEquipItem(BoundObject))
		{
			EquipSlot(EquipmentSlot->GetSlotName(), BoundObject);
			break;
		}
	}
}
