// Fill out your copyright notice in the Description page of Project Settings.

#include "ArmorObject.h"

#include "ArmorActor.h"
#include "Character/CharacterInventoryComponent.h"
#include "Net/UnrealNetwork.h"

void UArmorInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, ArmorData, COND_OwnerOnly);
}

bool UArmorInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	return bReplicateSomething;
}

void UArmorInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                     const UItemPredictedData* PredictedData)
{
	Super::SetupProperties(NewItemId, Definition, PredictedData);

	if (auto ArmorDefinition = Cast<UArmorDefinition>(Definition))
	{
		// Logic
		
		if (auto ArmorPredictedData = Cast<UArmorPredictedData>(PredictedData))
		{
			// Predicted data
		}
	}
}

void UArmorInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	Super::SetupProperties(NewItemId, Definition, Instance);
	
	if (auto ArmorInstance = Cast<UArmorInstance>(Instance))
	{
		// Logic
	}
}

void UArmorObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UArmorObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
}

void UArmorObject::OnBindItemActor()
{
	Super::OnBindItemActor();
}

void UArmorObject::OnUnbindItemActor(AItemActor* PrevItemActor)
{
	Super::OnUnbindItemActor(PrevItemActor);
}

bool UArmorObject::IsCorrespondsTo(const UItemObject* OtherItemObject) const
{
	return Super::IsCorrespondsTo(OtherItemObject);
}

USkeletalMesh* UArmorObject::GetVisual() const
{
	return nullptr; // TODO;
}

AArmorActor* UArmorObject::GetArmorActor() const
{
	return GetBoundActor<AArmorActor>();
}

UArmorInstance* UArmorObject::GetArmorInstance() const
{
	return GetItemInstance<UArmorInstance>();
}
