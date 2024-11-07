// Fill out your copyright notice in the Description page of Project Settings.

#include "SlotContainerInterface.h"

void ISlotContainerInterface::EquipSlot(const FString& SlotName, uint32 ItemId)
{
}

void ISlotContainerInterface::UnequipSlot(const FString& SlotName)
{
}

bool ISlotContainerInterface::CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject)
{
	return false;
}

UEquipmentSlot* ISlotContainerInterface::FindEquipmentSlot(const FString& SlotName) const
{
	return nullptr;
}
