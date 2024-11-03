// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/CharacterInventoryComponent.h"
#include "CharacterEquipmentWidget.generated.h"

UCLASS()
class STALKER_API UCharacterEquipmentWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEquipmentSlotWidget> ArmorSlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> PrimarySlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> SecondarySlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> DetectorSlot;
	
public:
	void SetupCharacterEquipment(UCharacterInventoryComponent* CharInventoryComp) const;
	void ClearCharacterEquipment();

	TArray<UEquipmentSlotWidget*> GetAllSlots() const;
};
