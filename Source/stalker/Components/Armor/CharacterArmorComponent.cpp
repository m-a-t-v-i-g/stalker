// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArmorComponent.h"
#include "ItemObject.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Inventory/EquipmentSlot.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacterArmorComponent::UCharacterArmorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterArmorComponent::PreInitializeArmor()
{
	if (auto CharacterInventoryComp = GetOwner()->GetComponentByClass<UCharacterInventoryComponent>())
	{
		if (auto ArmorSlot = CharacterInventoryComp->FindEquipmentSlot(ArmorSlotName))
		{
			ArmorSlot->OnSlotChanged.AddUObject(this, &UCharacterArmorComponent::OnArmorSlotChanged);
		}
	}
}

void UCharacterArmorComponent::PostInitializeArmor()
{
	
}

void UCharacterArmorComponent::OnArmorSlotChanged(UItemObject* ItemObject, bool bModified, bool bEquipped)
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
