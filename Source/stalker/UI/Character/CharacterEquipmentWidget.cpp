// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterEquipmentWidget.h"
#include "EquipmentSlotWidget.h"
#include "Attributes/ResistanceAttributeSet.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

void UCharacterEquipmentWidget::SetupCharacterEquipment(UAbilitySystemComponent* AbilityComp, UEquipmentComponent* EquipmentComp,
                                                        UInventoryManagerComponent* InventoryManager)
{
	ArmorSlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	PrimarySlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	SecondarySlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	DetectorSlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);

	AbilityComponentRef = AbilityComp;
	if (AbilityComponentRef.IsValid())
	{
		ResistanceAttributeSet = Cast<UResistanceAttributeSet>(AbilityComponentRef->GetAttributeSet(UResistanceAttributeSet::StaticClass()));
		if (!ResistanceAttributeSet.IsValid())
		{
			return;
		}

		auto& BulletResistanceAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
			ResistanceAttributeSet->GetBulletResistanceAttribute());
		auto& BlastResistanceAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
			ResistanceAttributeSet->GetBlastResistanceAttribute());

		BulletResistanceDelHandle = BulletResistanceAttrDelegate.AddUObject(this, &UCharacterEquipmentWidget::OnBulletResistanceUpdated);
		BlastResistanceDelHandle = BlastResistanceAttrDelegate.AddUObject(this, &UCharacterEquipmentWidget::OnBlastResistanceUpdated);

		ForceUpdateResistanceText();
	}
}

void UCharacterEquipmentWidget::ClearCharacterEquipment()
{
	ArmorSlot->ClearEquipmentSlot();
	PrimarySlot->ClearEquipmentSlot();
	SecondarySlot->ClearEquipmentSlot();
	DetectorSlot->ClearEquipmentSlot();

	if (AbilityComponentRef.IsValid())
	{
		auto& BulletResistanceAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
			ResistanceAttributeSet->GetBulletResistanceAttribute());
		auto& BlastResistanceAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
			ResistanceAttributeSet->GetBlastResistanceAttribute());

		BulletResistanceAttrDelegate.RemoveAll(this);
		BlastResistanceAttrDelegate.RemoveAll(this);
		
		AbilityComponentRef.Reset();
	}
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

void UCharacterEquipmentWidget::OnBulletResistanceUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	FText AsNumber = UKismetTextLibrary::Conv_IntToText(AttributeChangeData.NewValue, false, true, 1, 1);
	BulletResistanceText->SetText(AsNumber);
}

void UCharacterEquipmentWidget::OnBlastResistanceUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	FText AsNumber = UKismetTextLibrary::Conv_IntToText(AttributeChangeData.NewValue, false, true, 1, 1);
	BlastResistanceText->SetText(AsNumber);
}

void UCharacterEquipmentWidget::ForceUpdateResistanceText()
{
	FOnAttributeChangeData BulletResValueData;
	BulletResValueData.NewValue = ResistanceAttributeSet->GetBulletResistance();

	FOnAttributeChangeData BlastResValueData;
	BlastResValueData.NewValue = ResistanceAttributeSet->GetBlastResistance();

	OnBulletResistanceUpdated(BulletResValueData);
	OnBlastResistanceUpdated(BlastResValueData);
}
