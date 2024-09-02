// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterEquipmentWidget.h"
#include "EquipmentSlotWidget.h"

void UCharacterEquipmentWidget::InitializeCharacterEquipment(UCharacterInventoryComponent* CharInventoryComp) const
{
	ArmorSlot->SetupEquipmentSlot(CharInventoryComp);
	PrimarySlot->SetupEquipmentSlot(CharInventoryComp);
	SecondarySlot->SetupEquipmentSlot(CharInventoryComp);
	DetectorSlot->SetupEquipmentSlot(CharInventoryComp);
}

TArray<UEquipmentSlotWidget*> UCharacterEquipmentWidget::GetAllSlotWidgets() const
{
	TArray<UEquipmentSlotWidget*> Array;
	Array.Add(ArmorSlot);
	Array.Add(PrimarySlot);
	Array.Add(SecondarySlot);
	Array.Add(DetectorSlot);
	return Array;
}
