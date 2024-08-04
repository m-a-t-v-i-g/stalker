// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"
#include "Items/ItemActor.h"
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
	if (!ItemObject) return;

	if (auto Slot = &WeaponSlots[ActiveSlot])
	{
		Slot->ArmedItemActor = SpawnWeapon(Slot, ItemObject);
		Slot->ArmedItemObject = ItemObject;
	}
}

void UWeaponComponent::DisarmSlot(const FString& SlotName)
{
	if (auto Slot = &WeaponSlots[ActiveSlot])
	{
		Slot->ArmedItemObject = nullptr;
	}
}

void UWeaponComponent::Attack()
{
	
}

AItemActor* UWeaponComponent::SpawnWeapon(const FWeaponSlot* WeaponSlot, const UItemObject* ItemObject) const
{
	AItemActor* SpawnedWeapon = nullptr;
	if (ItemObject && ItemObject->GetActorClass())
	{
		FTransform SpawnTransform = GetOwner()->GetActorTransform();
		FAttachmentTransformRules AttachmentRules {EAttachmentRule::SnapToTarget, true};
		
		SpawnedWeapon = GetWorld()->SpawnActorDeferred<AItemActor>(ItemObject->GetActorClass(), SpawnTransform,
		                                                           GetOwner(), GetOwner<APawn>(),
		                                                           ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		// TODO: SpawnedWeapon->AttachToComponent(nullptr, AttachmentRules, WeaponSlot->GetAttachmentSocketName());
		SpawnedWeapon->FinishSpawning(SpawnTransform);
	}
	return SpawnedWeapon;
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
