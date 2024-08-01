// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponObject.h"
#include "Components/Inventory/CharacterInventoryComponent.h"
#include "Net/UnrealNetwork.h"

void UWeaponObject::Use(UObject* Source)
{
	Super::Use(Source);
	
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
