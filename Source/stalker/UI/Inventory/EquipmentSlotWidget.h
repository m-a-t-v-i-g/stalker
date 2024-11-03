// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemsLibrary.h"
#include "EquipmentSlotWidget.generated.h"

class UEquipmentSlot;
class UItemObject;
class UItemWidget;

UCLASS()
class STALKER_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnEquippedItemOperationSignature OnItemWidgetDoubleClick;
	
	void SetupEquipmentSlot(UObject* SlotContainerReference);
	void ClearEquipmentSlot();

	bool IsSlotOccupied() const;
	bool IsSlotEmpty() const;

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	bool bVerticalSlot = false;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> SlotCanvas;

	void OnSlotChanged(UItemObject* BoundObject, bool bModified, bool bEquipped);

	void UpdateCanvas();

	void OnDoubleClick(UItemObject* ClickedItem);
	void OnDragItem(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                UDragDropOperation* InOperation);

	UFUNCTION()
	void OnDragItemCancelled(UDragDropOperation* InOperation);
	
	UFUNCTION()
	void OnDropItem(UDragDropOperation* InOperation);
	
	UItemWidget* CreateItemWidget(UItemObject* ItemObject);

private:
	TWeakObjectPtr<UEquipmentSlot> EquipmentSlotRef;
	TWeakObjectPtr<UObject> SlotContainerRef;
};
