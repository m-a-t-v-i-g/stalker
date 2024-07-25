// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterInventoryComponent.h"
#include "EquipmentSlot.h"
#include "Engine/ActorChannel.h"
#include "Interactive/Items/ItemObject.h"
#include "Library/Items/ItemsFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCharacterInventoryComponent, EquipmentSlots, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCharacterInventoryComponent, EquippedItems, COND_None);
}

bool UCharacterInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                       FReplicationFlags* RepFlags)
{
	bool ReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UEquipmentSlot* EachSlot : EquipmentSlots)
	{
		ReplicateSomething |= Channel->ReplicateSubobject(EachSlot, *Bunch, *RepFlags);
	}
	for (UItemObject* EachItem : EquippedItems)
	{
		ReplicateSomething |= Channel->ReplicateSubobject(EachItem, *Bunch, *RepFlags);
	}
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UCharacterInventoryComponent::InitializeContainer()
{
	if (EquipmentSlotSpecs.Num() > 0)
	{
		for (int i = 0; i < EquipmentSlotSpecs.Num(); i++)
		{
			auto NewEquipmentSlot = NewObject<UEquipmentSlot>(this);
			NewEquipmentSlot->SetupSlot(&EquipmentSlotSpecs[i]);
			
			EquipmentSlots.Add(NewEquipmentSlot);
		}
	}
	Super::InitializeContainer();
}

void UCharacterInventoryComponent::AddStartingData()
{
	Super::AddStartingData();

	for (auto EachSlotSpec : EquipmentSlotSpecs)
	{
		if (EachSlotSpec.StartingData.IsValid())
		{
			auto Slot = FindEquipmentSlotByName(EachSlotSpec.SlotName);
			if (!Slot) continue;

			FItemData ItemData;
			ItemData.ItemRow = EachSlotSpec.StartingData.ItemRow;
			ItemData.ItemParams.Amount = 1;

			if (auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemData))
			{
				EquipSlot(EachSlotSpec.SlotName, ItemObject->GetItemTag(), ItemObject);
			}
			// TODO: Generate items and equip at slots
		}
	}
}

void UCharacterInventoryComponent::EquipSlot(const FString& SlotName, const FGameplayTag& ItemTag,
                                             UItemObject* BoundObject)
{
	if (auto Slot = FindEquipmentSlotByName(SlotName))
	{
		Slot->EquipSlot(ItemTag, BoundObject);

		if (!EquippedItems.Contains(BoundObject))
		{
			EquippedItems.Add(BoundObject);
		}
	}
	
	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_EquipSlot(SlotName, ItemTag, BoundObject);
	}
}

void UCharacterInventoryComponent::Server_EquipSlot_Implementation(const FString& SlotName, const FGameplayTag& ItemTag,
                                                                   UItemObject* BoundObject)
{
	EquipSlot(SlotName, ItemTag, BoundObject);
}

void UCharacterInventoryComponent::UnEquipSlot(const FString& SlotName, UItemObject* OldBoundObject)
{
	if (auto Slot = FindEquipmentSlotByName(SlotName))
	{
		Slot->UnEquipSlot();

		if (EquippedItems.Contains(OldBoundObject))
		{
			EquippedItems.Remove(OldBoundObject);
		}
	}
	
	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_UnEquipSlot(SlotName, OldBoundObject);
	}
}

void UCharacterInventoryComponent::Server_UnEquipSlot_Implementation(const FString& SlotName, UItemObject* OldBoundObject)
{
	UnEquipSlot(SlotName, OldBoundObject);
}

UEquipmentSlot* UCharacterInventoryComponent::FindEquipmentSlotByName(const FString& SlotName) const
{
	UEquipmentSlot* FoundSlot = nullptr;
	if (EquipmentSlots.Num() > 0)
	{
		FoundSlot = *EquipmentSlots.FindByPredicate([&, SlotName] (const UEquipmentSlot* Slot)
		{
			return Slot->GetSlotName() == SlotName;
		});
	}
	return FoundSlot;
}
