// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponComponent.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetIsReplicatedByDefault(true);
}

void UWeaponComponent::ArmSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (!ItemObject)
	{
		return;
	}

	if (auto Slot = FindWeaponSlot(SlotName))
	{
		Slot->ArmedObject = ItemObject;
	}
}

void UWeaponComponent::DisarmSlot(const FString& SlotName)
{
	if (auto Slot = FindWeaponSlot(SlotName))
	{
		if (!Slot->IsArmed())
		{
			return;
		}
		Slot->ArmedObject = nullptr;
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
	return true;
}

bool UWeaponComponent::IsArmed() const
{
	return true;
}

FWeaponSlot* UWeaponComponent::FindWeaponSlot(const FString& SlotName)
{
	for (FWeaponSlot& Slot : WeaponSlots)
	{
		if (Slot.SlotName == SlotName)
		{
			return &Slot;
		}
	}
	return nullptr;
}

bool UWeaponComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UWeaponComponent::IsAutonomousProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UWeaponComponent::IsSimulatedProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_SimulatedProxy;
}
