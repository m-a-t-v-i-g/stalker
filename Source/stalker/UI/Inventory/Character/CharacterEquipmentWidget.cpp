// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterEquipmentWidget.h"
#include "Character/CharacterInventoryComponent.h"
#include "Inventory/EquipmentSlotWidget.h"

void UCharacterEquipmentWidget::SetupCharacterEquipment(UCharacterInventoryComponent* CharInventoryComp) const
{
	ArmorSlot->SetupEquipmentSlot(CharInventoryComp);
	PrimarySlot->SetupEquipmentSlot(CharInventoryComp);
	SecondarySlot->SetupEquipmentSlot(CharInventoryComp);
	DetectorSlot->SetupEquipmentSlot(CharInventoryComp);
}

void UCharacterEquipmentWidget::ClearCharacterEquipment() const
{
	ArmorSlot->ClearEquipmentSlot();
	PrimarySlot->ClearEquipmentSlot();
	SecondarySlot->ClearEquipmentSlot();
	DetectorSlot->ClearEquipmentSlot();
}

TArray<UEquipmentSlotWidget*> UCharacterEquipmentWidget::GetAllSlots() const
{
	TArray<UEquipmentSlotWidget*> Array;
	Array.Add(ArmorSlot);
	Array.Add(PrimarySlot);
	Array.Add(SecondarySlot);
	Array.Add(DetectorSlot);
	return Array;
}
