// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "InventorySystemCore.h"

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsAuthority())
	{
		for (UEquipmentSlot* Slot : EquipmentSlots)
		{
			Slot->AddStartingData();
		}
	}
}

void UEquipmentComponent::EquipSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (IsAuthority())
	{
		if (UEquipmentSlot* EquipmentSlot = FindEquipmentSlot(SlotName))
		{
			UInventorySystemCore::EquipSlot(EquipmentSlot, ItemObject);
		}
	}
}

void UEquipmentComponent::UnequipSlot(const FString& SlotName)
{
	if (IsAuthority())
	{
		if (UEquipmentSlot* EquipmentSlot = FindEquipmentSlot(SlotName))
		{
			UInventorySystemCore::UnequipSlot(EquipmentSlot);
		}
	}
}

void UEquipmentComponent::AddEquipmentSlot(UEquipmentSlot* NewSlot)
{
	EquipmentSlots.AddUnique(NewSlot);
}

bool UEquipmentComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UEquipmentComponent::CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject)
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

UEquipmentSlot* UEquipmentComponent::FindEquipmentSlot(const FString& SlotName) const
{
	if (!EquipmentSlots.IsEmpty())
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

TArray<UItemObject*> UEquipmentComponent::GetAllEquippedItems() const
{
	TArray<UItemObject*> EquippedItems;
	if (!EquipmentSlots.IsEmpty())
	{
		for (UEquipmentSlot* Slot : EquipmentSlots)
		{
			if (UItemObject* ItemObject = Slot->GetBoundObject())
			{
				EquippedItems.AddUnique(ItemObject);
			}
		}
	}
	return EquippedItems;
}
