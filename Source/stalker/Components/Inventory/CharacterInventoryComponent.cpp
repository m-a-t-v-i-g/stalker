// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterInventoryComponent.h"
#include "EquipmentSlot.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UCharacterInventoryComponent::UCharacterInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCharacterInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCharacterInventoryComponent, EquipmentSlots, COND_OwnerOnly);
}

bool UCharacterInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                       FReplicationFlags* RepFlags)
{
	bool ReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UEquipmentSlot* EachSlot : EquipmentSlots)
	{
		ReplicateSomething |= Channel->ReplicateSubobject(EachSlot, *Bunch, *RepFlags);
	}
	return Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
}

void UCharacterInventoryComponent::InitializeContainer()
{
	Super::InitializeContainer();

	if (EquipmentSlotSpecs.Num() > 0)
	{
		for (int i = 0; i < EquipmentSlotSpecs.Num(); i++)
		{
			auto NewEquipmentSlot = NewObject<UEquipmentSlot>(this);
			EquipmentSlots.Add(NewEquipmentSlot);
		}
	}
}

UEquipmentSlot* UCharacterInventoryComponent::FindEquipmentSlotByName(const FString& SlotName) const
{
	UEquipmentSlot* FoundSlot = *EquipmentSlots.FindByPredicate([&, SlotName] (const UEquipmentSlot* Slot)
	{
		return Slot->GetSlotName() == SlotName;
	});
	return FoundSlot;
}
