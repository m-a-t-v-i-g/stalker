// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlot.h"
#include "Interactive/Items/ItemObject.h"
#include "Net/UnrealNetwork.h"

void UEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquipmentSlot, CategoryTags,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, SlotName,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, ItemTag,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, BoundObject,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, bAvailable,		COND_OwnerOnly);
}

void UEquipmentSlot::SetupSlot(const FEquipmentSlotSpec* EquipmentSlotSpec)
{
	CategoryTags = EquipmentSlotSpec->CategoryTags;
	SlotName = EquipmentSlotSpec->SlotName;
}

bool UEquipmentSlot::EquipSlot(const FGameplayTag& InItemTag, UItemObject* BindObject)
{
	bool bWasEquipped = false;
	if (BindObject)
	{
		ItemTag = InItemTag;
		BoundObject = BindObject;
		bWasEquipped = true;
		
		OnSlotChanged.Broadcast(InItemTag, BindObject);
	}
	return bWasEquipped;
}

void UEquipmentSlot::UnEquipSlot()
{
	if (BoundObject.IsValid())
	{
		ItemTag = FGameplayTag::EmptyTag;
		BoundObject.Reset();
		
		OnSlotChanged.Broadcast(ItemTag, BoundObject.Get());
	}
}

bool UEquipmentSlot::CanEquipItem(const UItemObject* ItemObject) const
{
	return ItemObject->GetItemTag().MatchesAny(CategoryTags);
}

void UEquipmentSlot::UpdateSlot() const
{
	OnSlotChanged.Broadcast(ItemTag, BoundObject.Get());
}
