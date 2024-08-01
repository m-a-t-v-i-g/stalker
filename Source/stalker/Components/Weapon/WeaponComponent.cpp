// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"
#include "Interactive/Items/ItemActor.h"
#include "Interactive/Items/ItemObject.h"
#include "Interactive/Items/Weapons/WeaponObject.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponComponent::PreInitializeWeapon()
{
	if (WeaponSlotSpecs.Num() > 0)
	{
		for (uint8 i = 0; i < WeaponSlotSpecs.Num(); i++)
		{
			FWeaponSlotHandle SlotHandle {i, WeaponSlotSpecs[i]};
			WeaponSlots.Add(SlotHandle);
		}
	}
}

void UWeaponComponent::PostInitializeWeapon()
{
	
}

bool UWeaponComponent::ActivateSlot(const FString& SlotName)
{
	for (uint8 i = 0; i < WeaponSlots.Num(); i++)
	{
		if (WeaponSlots[i].GetSlotName() == SlotName)
		{
			ActiveSlot = i;
			return true;
		}
	}
	return false;
}

bool UWeaponComponent::ActivateSlot(uint8 SlotIndex)
{
	if (WeaponSlots.IsValidIndex(SlotIndex))
	{
		ActiveSlot = SlotIndex;
		return true;
	}
	return false;
}

void UWeaponComponent::ArmSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (auto Slot = FindWeaponSlot(SlotName))
	{
		Slot->ArmedItemActor = SpawnWeapon(ItemObject);
		Slot->ArmedItemObject = ItemObject;
	}
}

void UWeaponComponent::Attack()
{
	
}

AItemActor* UWeaponComponent::SpawnWeapon(const UItemObject* ItemObject) const
{
	AItemActor* SpawnedWeapon = nullptr;
	if (ItemObject && ItemObject->GetActorClass())
	{
		FTransform SpawnTransform;
		FActorSpawnParameters SpawnParams;
		SpawnedWeapon = GetWorld()->SpawnActorDeferred<AItemActor>(ItemObject->GetActorClass(), SpawnTransform,
		                                                           GetOwner(), GetOwner<APawn>(),
		                                                           ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		SpawnedWeapon->FinishSpawning(SpawnTransform);
	}
	return SpawnedWeapon;
}

FWeaponSlotHandle* UWeaponComponent::FindWeaponSlot(const FString& SlotName)
{
	FWeaponSlotHandle* FoundSlot = nullptr;
	if (WeaponSlots.Num() > 0)
	{
		FoundSlot = WeaponSlots.FindByPredicate([&, SlotName] (const FWeaponSlotHandle& Slot)
		{
			return Slot.GetSlotName() == SlotName;
		});
	}
	return FoundSlot;
}
