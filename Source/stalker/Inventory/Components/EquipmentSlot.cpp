// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlot.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UEquipmentSlot, BoundObject,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UEquipmentSlot, bAvailable,		COND_OwnerOnly);
}

bool UEquipmentSlot::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	if (BoundObject)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(BoundObject, *Bunch, *RepFlags);
		bReplicateSomething |= BoundObject->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	return bReplicateSomething;
}

void UEquipmentSlot::SetupEquipmentSlot(FString InSlotName, FGameplayTagContainer InSlotTags)
{
	SlotName = InSlotName;
	CategoryTags = InSlotTags;
}

void UEquipmentSlot::AddStartingData()
{
	if (StartingData.IsEmpty())
	{
		return;
	}
	
	int RandIndex = FMath::RandRange(0, StartingData.Num() - 1);
	
	if (StartingData.IsValidIndex(RandIndex))
	{
		const FItemStartingData& ItemData = StartingData[RandIndex];
		if (ItemData.IsValid())
		{
			if (CanEquipItem(ItemData.Definition))
			{
				UItemPredictedData* PredictedData = ItemData.bUsePredictedData ? ItemData.PredictedData : nullptr;
				if (UItemObject* ItemObject = UItemSystemCore::GenerateItemObject(
					GetWorld(), ItemData.Definition, PredictedData))
				{
					EquipSlot(ItemObject);
				}
			}
		}
	}
	
	StartingData.Empty();
}

void UEquipmentSlot::EquipSlot(UItemObject* BindObject)
{
	if (BindObject)
	{
		BoundObject = BindObject;
		BoundObject->SetEquipped();
		OnSlotChanged.Broadcast(FEquipmentSlotChangeData(BoundObject, true));
	}
}

void UEquipmentSlot::UnequipSlot()
{
	if (BoundObject)
	{
		OnSlotChanged.Broadcast(FEquipmentSlotChangeData(BoundObject, false));
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
		OnSlotChanged.Broadcast(FEquipmentSlotChangeData(ItemObject, IsEquipped()));
	}
}
