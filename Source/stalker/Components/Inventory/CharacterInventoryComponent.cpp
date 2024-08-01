// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterInventoryComponent.h"
#include "EquipmentSlot.h"
#include "Engine/ActorChannel.h"
#include "Interactive/Items/ItemObject.h"
#include "Kismet/KismetSystemLibrary.h"
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
	return ReplicateSomething;
}

void UCharacterInventoryComponent::PreInitializeContainer()
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
	Super::PreInitializeContainer();
}

void UCharacterInventoryComponent::AddStartingData()
{
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
				EquipSlot(EachSlotSpec.SlotName, ItemObject, false);
			}
		}
	}
	Super::AddStartingData();
}

void UCharacterInventoryComponent::TryEquipItem(UItemObject* BoundObject)
{
	for (auto EquipmentSlot : EquipmentSlots)
	{
		if (!IsValid(EquipmentSlot)) continue;

		if (EquipmentSlot->CanEquipItem(BoundObject))
		{
			EquipSlot(EquipmentSlot->GetSlotName(), BoundObject, true);
		}
	}
}

bool UCharacterInventoryComponent::EquipSlot(const FString& SlotName, UItemObject* BoundObject, bool bSubtractItem)
{
	check(BoundObject);
	
	if (auto Slot = FindEquipmentSlotByName(SlotName))
	{
		if (auto SlotObject = Slot->GetBoundObject())
		{
			if (SlotObject->IsSimilar(BoundObject))
			{
				UKismetSystemLibrary::PrintString(
					this, FString("Equipped item is identical to the item you are trying to put on!"), true, false,
					FLinearColor::Red, 5.0f);
				return false;
			}
		}
	}
	Server_EquipSlot(SlotName, BoundObject, bSubtractItem);
	return true;
}

void UCharacterInventoryComponent::Server_EquipSlot_Implementation(const FString& SlotName, UItemObject* BoundObject,
                                                                   bool bSubtractItem)
{
	check(BoundObject);
	
	if (auto Slot = FindEquipmentSlotByName(SlotName))
	{
		if (auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(BoundObject))
		{
			if (bSubtractItem)
			{
				SubtractOrRemoveItem(BoundObject);
			}
			
			if (Slot->IsEquipped())
			{
				UnEquipSlot(SlotName, true);
			}
			
			ItemObject->SetEquippedMode();
			Slot->EquipSlot(ItemObject);

			if (!EquippedItems.Contains(ItemObject))
			{
				EquippedItems.Add(ItemObject);
			}
		}
	}
}

void UCharacterInventoryComponent::UnEquipSlot(const FString& SlotName, bool bTryAddItem)
{
	Server_UnEquipSlot(SlotName, bTryAddItem);
}

void UCharacterInventoryComponent::Server_UnEquipSlot_Implementation(const FString& SlotName, bool bTryAddItem)
{
	if (auto Slot = FindEquipmentSlotByName(SlotName))
	{
		if (Slot->IsEquipped())
		{
			auto OldBoundObject = Slot->GetBoundObject();
			check(OldBoundObject);

			Slot->UnEquipSlot();
			
			if (bTryAddItem)
			{
				FindAvailablePlace(OldBoundObject);
			}

			if (EquippedItems.Contains(OldBoundObject))
			{
				EquippedItems.Remove(OldBoundObject);
			}
		}
	}
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
