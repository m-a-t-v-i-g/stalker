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
	if (!StartingData.IsValid())
	{
		return;
	}
	
	if (CanEquipItem(StartingData.Definition))
	{
		if (UItemObject* ItemObject = UItemsFunctionLibrary::GenerateItemObject(
			GetWorld(), StartingData.Definition, StartingData.PredictedData))
		{
			EquipSlot(ItemObject);
		}
	}
	StartingData.Clear();
}

void UEquipmentSlot::EquipSlot(UItemObject* BindObject)
{
	if (BindObject)
	{
		BoundObject = BindObject;
		OnSlotChanged.Broadcast(FUpdatedSlotData(BoundObject, true));
	}
}

void UEquipmentSlot::UnequipSlot()
{
	if (BoundObject)
	{
		OnSlotChanged.Broadcast(FUpdatedSlotData(BoundObject, false));
		BoundObject = nullptr;
	}
}

bool UEquipmentSlot::CanEquipItem(const UItemDefinition* ItemDefinition) const
{
	return ItemDefinition->Tag.MatchesAny(CategoryTags);
}

void UEquipmentSlot::OnRep_BoundObject(UItemObject* PrevItemObject)
{
	if (UItemObject* ItemObject = IsEquipped() ? BoundObject.Get() : PrevItemObject)
	{
		OnSlotChanged.Broadcast(FUpdatedSlotData(ItemObject, IsEquipped()));
	}
}
