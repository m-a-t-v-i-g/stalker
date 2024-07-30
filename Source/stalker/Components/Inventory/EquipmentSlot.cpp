// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlot.h"
#include "Interactive/Items/ItemObject.h"
#include "Net/UnrealNetwork.h"

void UEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquipmentSlot, CategoryTags,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, SlotName,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, BoundObject,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, bAvailable,		COND_OwnerOnly);
}

void UEquipmentSlot::SetupSlot(const FEquipmentSlotSpec* EquipmentSlotSpec)
{
	CategoryTags = EquipmentSlotSpec->CategoryTags;
	SlotName = EquipmentSlotSpec->SlotName;
}

bool UEquipmentSlot::EquipSlot(UItemObject* BindObject)
{
	bool bWasEquipped = false;
	if (BindObject)
	{
		BoundObject = BindObject;
		bWasEquipped = true;
		
		OnSlotChanged.Broadcast(BindObject);
	}
	return bWasEquipped;
}

void UEquipmentSlot::UnEquipSlot()
{
	if (BoundObject.IsValid())
	{
		BoundObject.Reset();
		
		OnSlotChanged.Broadcast(BoundObject.Get());
	}
}

bool UEquipmentSlot::CanEquipItem(const UItemObject* ItemObject) const
{
	return ItemObject->GetItemTag().MatchesAny(CategoryTags);
}

void UEquipmentSlot::UpdateSlot() const
{
	OnSlotChanged.Broadcast(BoundObject.Get());
}

void UEquipmentSlot::OnRep_EquipmentSlot()
{
	UpdateSlot();
}
