// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Net/UnrealNetwork.h"

void UWeaponObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);

	if (auto CharInventory = Cast<UCharacterInventoryComponent>(Source))
	{
		CharInventory->TryEquipItem(this);
	}
}

void UWeaponObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(UWeaponObject, WeaponParams, COND_OwnerOnly);
}

FWeaponParams UWeaponObject::GetWeaponParams() const
{
	return WeaponParams;
}
