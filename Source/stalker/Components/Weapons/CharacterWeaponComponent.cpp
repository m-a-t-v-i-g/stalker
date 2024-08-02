// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "Characters/StalkerCharacter.h"
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
	StalkerCharacter = GetOwner<AStalkerCharacter>();
	if (!StalkerCharacter.Get()) return;
	
	if (auto CharacterInventoryComp = StalkerCharacter->GetComponentByClass<UCharacterInventoryComponent>())
	{
		CharacterInventory = CharacterInventoryComp;
		
		if (auto KnifeSlot = CharacterInventory->FindEquipmentSlot(KnifeSlotName))
		{
			KnifeSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnKnifeSlotChanged);
		}
		
		if (auto MainSlot = CharacterInventory->FindEquipmentSlot(MainSlotName))
		{
			MainSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnMainSlotChanged);
		}
		
		if (auto SecondarySlot = CharacterInventory->FindEquipmentSlot(SecondarySlotName))
		{
			SecondarySlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSecondarySlotChanged);
		}
		
		if (auto GrenadeSlot = CharacterInventory->FindEquipmentSlot(GrenadeSlotName))
		{
			GrenadeSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnGrenadeSlotChanged);
		}
		
		if (auto BinocularSlot = CharacterInventory->FindEquipmentSlot(BinocularSlotName))
		{
			BinocularSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnBinocularSlotChanged);
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

void UCharacterWeaponComponent::OnKnifeSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{
		ArmSlot(KnifeSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Knife equipped!"), true, false);
	}
	else
	{
		DisarmSlot(KnifeSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Knife unequipped!"), true, false);
	}
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

void UCharacterWeaponComponent::OnGrenadeSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(GrenadeSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Grenade equipped!"), true, false);
	}
	else
	{
		DisarmSlot(GrenadeSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Grenade unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnBinocularSlotChanged(UItemObject* ItemObject, bool bModified)
{
	
}

void UCharacterWeaponComponent::OnBoltSlotChanged(UItemObject* ItemObject, bool bModified)
{
	
}

void UCharacterWeaponComponent::OnDetectorSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(DetectorSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Detector equipped!"), true, false);
	}
	else
	{
		DisarmSlot(DetectorSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Detector unequipped!"), true, false);
	}
}
