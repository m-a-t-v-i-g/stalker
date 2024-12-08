// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterEquipmentWidget.h"
#include "EquipmentSlotWidget.h"

void UCharacterEquipmentWidget::SetupCharacterEquipment(UEquipmentComponent* EquipmentComp,
                                                        UInventoryManagerComponent* InventoryManager) const
{
	ArmorSlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	PrimarySlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	SecondarySlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	DetectorSlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
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
