// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSlotWidget.generated.h"

class UCharacterInventoryComponent;
class UEquipmentSlot;
class UItemObject;

UCLASS()
class STALKER_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> SlotCanvas;

private:
	TWeakObjectPtr<UEquipmentSlot> OwnSlot;
	
public:
	void SetupEquipmentSlot(const UCharacterInventoryComponent* CharInventoryComp);

protected:
	void OnSlotEquipped(const FGameplayTag& ItemTag, UItemObject* BoundObject);
};
