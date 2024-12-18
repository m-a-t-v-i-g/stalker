// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterEquipmentWidget.h"
#include "EquipmentSlotWidget.h"
#include "Attributes/ResistanceAttributeSet.h"
#include "Components/TextBlock.h"

void UCharacterEquipmentWidget::SetupCharacterEquipment(UAbilitySystemComponent* AbilityComp,
                                                        UEquipmentComponent* EquipmentComp,
                                                        UInventoryManagerComponent* InventoryManager)
{
	ArmorSlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	PrimarySlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	SecondarySlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);
	DetectorSlot->SetupEquipmentSlot(EquipmentComp, InventoryManager);

	AbilityComponentRef = AbilityComp;

	if (!AbilityComponentRef.IsValid())
	{
		return;
	}
	
	ResistanceAttribute = Cast<UResistanceAttributeSet>(AbilityComponentRef->GetAttributeSet(UResistanceAttributeSet::StaticClass()));
	if (!ResistanceAttribute.IsValid())
	{
		return;
	}

	auto& BulletResistanceAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
		ResistanceAttribute->GetBulletResistanceAttribute());
	auto& BlastResistanceAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
		ResistanceAttribute->GetBlastResistanceAttribute());
	
	BulletResistanceAttrDelegate.AddUObject(this, &UCharacterEquipmentWidget::OnBulletResistanceUpdated);
	BlastResistanceAttrDelegate.AddUObject(this, &UCharacterEquipmentWidget::OnBlastResistanceUpdated);

	ForceUpdateResistanceText();
}

void UCharacterEquipmentWidget::ClearCharacterEquipment()
{
	ArmorSlot->ClearEquipmentSlot();
	PrimarySlot->ClearEquipmentSlot();
	SecondarySlot->ClearEquipmentSlot();
	DetectorSlot->ClearEquipmentSlot();

	AbilityComponentRef.Reset();
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
	FString ValueString = FString::Printf(TEXT("%f"), AttributeChangeData.NewValue);
	BulletResistanceText->SetText(FText::FromString(ValueString));
}

void UCharacterEquipmentWidget::OnBlastResistanceUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	FString ValueString = FString::Printf(TEXT("%f"), AttributeChangeData.NewValue);
	BlastResistanceText->SetText(FText::FromString(ValueString));
}

void UCharacterEquipmentWidget::ForceUpdateResistanceText()
{
	FOnAttributeChangeData BulletResValueData;
	BulletResValueData.NewValue = ResistanceAttribute->GetBulletResistance();

	FOnAttributeChangeData BlastResValueData;
	BlastResValueData.NewValue = ResistanceAttribute->GetBlastResistance();

	OnBulletResistanceUpdated(BulletResValueData);
	OnBlastResistanceUpdated(BlastResValueData);
}
