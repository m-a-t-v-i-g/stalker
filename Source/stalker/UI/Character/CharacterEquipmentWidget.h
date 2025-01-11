// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterEquipmentWidget.generated.h"

class UAbilitySystemComponent;
class UEquipmentComponent;
class UInventoryManagerComponent;
class UResistanceAttributeSet;

UCLASS()
class STALKER_API UCharacterEquipmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupCharacterEquipment(UAbilitySystemComponent* AbilityComp, UEquipmentComponent* EquipmentComp,
	                             UInventoryManagerComponent* InventoryManager);
	void ClearCharacterEquipment();
	
	TArray<class UEquipmentSlotWidget*> GetAllSlots() const;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> ArmorSlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> PrimarySlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> SecondarySlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEquipmentSlotWidget> DetectorSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> BulletResistanceText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BlastResistanceText;

	void OnBulletResistanceUpdated(const struct FOnAttributeChangeData& AttributeChangeData);
	void OnBlastResistanceUpdated(const FOnAttributeChangeData& AttributeChangeData);

	void ForceUpdateResistanceText();
	
private:
	FDelegateHandle BulletResistanceDelHandle;
	FDelegateHandle BlastResistanceDelHandle;

	TWeakObjectPtr<UAbilitySystemComponent> AbilityComponentRef;
	TWeakObjectPtr<const UResistanceAttributeSet> ResistanceAttributeSet;
};
