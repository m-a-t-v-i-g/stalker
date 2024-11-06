// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlot.h"
#include "ItemObject.h"
#include "Items/ItemsFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

void UEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquipmentSlot, BoundObject,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, bAvailable,		COND_OwnerOnly);
}

void UEquipmentSlot::AddStartingData()
{
	if (auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(GetWorld(), StartingData.Definition,
	                                                                StartingData.PredictedData))
	{
		if (!EquipSlot(ItemObject))
		{
			ItemObject->MarkAsGarbage();
		}
	}
	StartingData.Clear();
}

bool UEquipmentSlot::EquipSlot(UItemObject* BindObject)
{
	if (BindObject)
	{
		BoundObject = BindObject;
		OnSlotChanged.Broadcast(BindObject, true, true);
		return true;
	}
	return false;
}

void UEquipmentSlot::UnEquipSlot()
{
	if (BoundObject)
	{
		OnSlotChanged.Broadcast(BoundObject, true, false);
		BoundObject = nullptr;
	}
}

bool UEquipmentSlot::CanEquipItem(const UItemObject* ItemObject) const
{
	if (ItemObject)
	{
		return ItemObject->GetItemTag().MatchesAny(CategoryTags);
	}
	return false;
}

void UEquipmentSlot::UpdateSlot(bool bModified) const
{
	OnSlotChanged.Broadcast(BoundObject, bModified, IsEquipped());
}

void UEquipmentSlot::OnRep_EquipmentSlot(UItemObject* PrevItemObject)
{
	UpdateSlot(PrevItemObject != BoundObject);
}
