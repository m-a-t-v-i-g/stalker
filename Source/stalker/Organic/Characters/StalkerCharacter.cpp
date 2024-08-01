// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "Components/Inventory/CharacterInventoryComponent.h"
#include "Components/Inventory/EquipmentSlot.h"
#include "Components/Weapon/WeaponComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Kismet/KismetSystemLibrary.h"

FName AStalkerCharacter::WeaponComponentName {"CharWeaponComp"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.Get().SetDefaultSubobjectClass<UCharacterInventoryComponent>(InventoryComponentName))
{
	CharacterWeapon = CreateDefaultSubobject<UWeaponComponent>(WeaponComponentName);
	
	PrimaryActorTick.bCanEverTick = true;
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (auto CharacterInventory = GetInventoryComponent<UCharacterInventoryComponent>())
	{
		CharacterInventory->PreInitializeContainer();

		if (auto ArmorSlot = CharacterInventory->FindEquipmentSlot(ArmorSlotName))
		{
			ArmorSlot->OnSlotChanged.AddUObject(this, &AStalkerCharacter::OnArmorSlotChanged);
		}
		
		if (auto MainSlot = CharacterInventory->FindEquipmentSlot(MainSlotName))
		{
			MainSlot->OnSlotChanged.AddUObject(this, &AStalkerCharacter::OnMainSlotChanged);
		}
		
		if (auto SecondarySlot = CharacterInventory->FindEquipmentSlot(SecondarySlotName))
		{
			SecondarySlot->OnSlotChanged.AddUObject(this, &AStalkerCharacter::OnSecondarySlotChanged);
		}
		
		if (auto DetectorSlot = CharacterInventory->FindEquipmentSlot(DetectorSlotName))
		{
			DetectorSlot->OnSlotChanged.AddUObject(this, &AStalkerCharacter::OnDetectorSlotChanged);
		}

		CharacterInventory->PostInitializeContainer();
	}
}

void AStalkerCharacter::OnArmorSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{
		UKismetSystemLibrary::PrintString(this, FString("Armor equipped!"), true, false);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString("Armor unequipped!"), true, false);
	}
}

void AStalkerCharacter::OnMainSlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{
		UKismetSystemLibrary::PrintString(this, FString("Rifle equipped!"), true, false);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString("Rifle unequipped!"), true, false);
	}
}

void AStalkerCharacter::OnSecondarySlotChanged(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		UKismetSystemLibrary::PrintString(this, FString("Pistol equipped!"), true, false);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString("Pistol unequipped!"), true, false);
	}
}

void AStalkerCharacter::OnDetectorSlotChanged(UItemObject* ItemObject, bool bModified)
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
