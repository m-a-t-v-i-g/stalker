// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Inventory/EquipmentSlot.h"
#include "Items/ItemObject.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacterWeaponComponent::UCharacterWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterWeaponComponent::PreInitializeWeapon()
{
	if (auto CharacterInventoryComp = GetOwner()->GetComponentByClass<UCharacterInventoryComponent>())
	{
		CharacterInventory = CharacterInventoryComp;
		
		if (auto MainSlot = CharacterInventory->FindEquipmentSlot(MainSlotName))
		{
			MainSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnMainSlotChanged);
		}
		
		if (auto SecondarySlot = CharacterInventory->FindEquipmentSlot(SecondarySlotName))
		{
			SecondarySlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSecondarySlotChanged);
		}
		
		if (auto DetectorSlot = CharacterInventory->FindEquipmentSlot(DetectorSlotName))
		{
			DetectorSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnDetectorSlotChanged);
		}
	}
	Super::PreInitializeWeapon();
}

void UCharacterWeaponComponent::PostInitializeWeapon()
{
	Super::PostInitializeWeapon();
}

void UCharacterWeaponComponent::OnMainSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{
		ArmSlot(MainSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Rifle equipped!"), true, false);
	}
	else
	{
		DisarmSlot(MainSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Rifle unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnSecondarySlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(SecondarySlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Pistol equipped!"), true, false);
	}
	else
	{
		DisarmSlot(SecondarySlotName);
		UKismetSystemLibrary::PrintString(this, FString("Pistol unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnDetectorSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		UKismetSystemLibrary::PrintString(this, FString("Detector equipped!"), true, false);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString("Detector unequipped!"), true, false);
	}
}
