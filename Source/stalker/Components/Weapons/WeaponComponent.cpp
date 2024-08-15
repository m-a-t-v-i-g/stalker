// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"
#include "Items/ItemObject.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::PreInitializeWeapon()
{
	if (WeaponSlotSpecs.Num() > 0)
	{
		for (uint8 i = 0; i < WeaponSlotSpecs.Num(); i++)
		{
			FWeaponSlot SlotHandle {i, WeaponSlotSpecs[i]};
			WeaponSlots.Add(SlotHandle);
		}
	}
}

void UWeaponComponent::PostInitializeWeapon()
{
}

void UWeaponComponent::ServerActivateSlot_Implementation(int8 SlotIndex)
{
	if (WeaponSlots.IsValidIndex(SlotIndex))
	{
		if (!WeaponSlots[SlotIndex].IsArmed())
		{
			return;
		}
	
		if (ActiveSlot == SlotIndex)
		{
			ServerDeactivateSlot();
		}
		else
		{
			ActiveSlot = SlotIndex;
			OnSlotActivated.Broadcast(WeaponSlots[SlotIndex].GetSlotName(), ActiveSlot, WeaponSlots[SlotIndex].ArmedItemObject);
		}
	}
}

void UWeaponComponent::ServerDeactivateSlot_Implementation()
{
	if (ActiveSlot >= 0)
	{
		OnSlotDeactivated.Broadcast(WeaponSlots[ActiveSlot].GetSlotName(), ActiveSlot,
		                            WeaponSlots[ActiveSlot].ArmedItemObject);
		ActiveSlot = -1;
	}
}

void UWeaponComponent::ArmSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (!ItemObject) return;

	if (auto Slot = FindWeaponSlot(SlotName))
	{
		Slot->ArmedItemObject = ItemObject;
	}
}

void UWeaponComponent::DisarmSlot(const FString& SlotName)
{
	if (auto Slot = FindWeaponSlot(SlotName))
	{
		if (!Slot->IsArmed()) return;
		
		Slot->ArmedItemObject = nullptr;
	}
}

bool UWeaponComponent::Attack()
{
	if (CanAttack())
	{
		OnAttack();
		return true;
	}
	return false;
}

void UWeaponComponent::OnAttack()
{
}

bool UWeaponComponent::CanAttack() const
{
	return WeaponSlots[ActiveSlot].IsArmed() || bAllowUnarmedAttack;
}

FWeaponSlot* UWeaponComponent::FindWeaponSlot(const FString& SlotName)
{
	FWeaponSlot* FoundSlot = nullptr;
	if (WeaponSlots.Num() > 0)
	{
		FoundSlot = WeaponSlots.FindByPredicate([&, SlotName] (const FWeaponSlot& Slot)
		{
			return Slot.GetSlotName() == SlotName;
		});
	}
	return FoundSlot;
}
