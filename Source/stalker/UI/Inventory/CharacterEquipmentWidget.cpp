// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterEquipmentWidget.h"
#include "EquipmentSlotWidget.h"

void UCharacterEquipmentWidget::InitializeCharacterEquipment(UCharacterInventoryComponent* CharInventoryComp) const
{
	ArmorSlot->SetupEquipmentSlot(CharInventoryComp);
	MainSlot->SetupEquipmentSlot(CharInventoryComp);
	SecondarySlot->SetupEquipmentSlot(CharInventoryComp);
	DetectorSlot->SetupEquipmentSlot(CharInventoryComp);
}
