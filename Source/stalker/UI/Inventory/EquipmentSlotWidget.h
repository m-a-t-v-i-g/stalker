// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Library/Items/ItemsLibrary.h"
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
	FOnEquippedItemOperationSignature OnItemWidgetDoubleClick;
	
	void SetupEquipmentSlot(UCharacterInventoryComponent* CharInventoryComp);

protected:
	void OnSlotChanged(UItemObject* BoundObject);

	void OnDoubleClick(UItemObject* ClickedItem);
	void OnCompleteDragOperation(UItemObject* DraggedItem);
	void OnReverseDragOperation(UItemObject* DraggedItem);
};
