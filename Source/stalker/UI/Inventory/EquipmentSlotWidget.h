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
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> SlotCanvas;

private:
	TWeakObjectPtr<UCharacterInventoryComponent> OwnEquipment;
	TWeakObjectPtr<UEquipmentSlot> OwnSlot;
	
public:
	void SetupEquipmentSlot(UCharacterInventoryComponent* CharInventoryComp);

protected:
	void OnSlotChanged(const FGameplayTag& ItemTag, UItemObject* BoundObject);
	
	void OnCompleteDragOperation(UItemObject* DraggedItem);
	void OnReverseDragOperation(UItemObject* DraggedItem);
};
