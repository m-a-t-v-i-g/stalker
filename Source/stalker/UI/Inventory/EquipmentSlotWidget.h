// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipmentSlotWidget.generated.h"

struct FEquipmentSlotChangeData;
class UEquipmentComponent;
class UEquipmentSlot;
class UInventoryManagerComponent;
class UItemObject;
class UItemWidget;

UCLASS()
class STALKER_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	TMulticastDelegate<void(UEquipmentSlot*)> OnItemWidgetDoubleClick;
	
	void SetupEquipmentSlot(UEquipmentComponent* EquipmentComp, UInventoryManagerComponent* InventoryManager);
	void ClearEquipmentSlot();

	bool IsSlotOccupied() const;
	bool IsSlotEmpty() const;

protected:
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	bool bVerticalSlot = false;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> SlotCanvas;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
							  UDragDropOperation* InOperation) override;
	
	void OnSlotUpdated(const FEquipmentSlotChangeData& UpdatedData);

	void OnDoubleClick(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent, UItemObject* ItemObject);
	void OnItemDrag(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                UDragDropOperation* InOperation);

	UFUNCTION()
	void OnDragItemCancelled(UDragDropOperation* InOperation);
	
	UFUNCTION()
	void OnDropItem(UDragDropOperation* InOperation);
	
	UItemWidget* CreateItemWidget(UItemObject* ItemObject);

private:
	TWeakObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	TWeakObjectPtr<UEquipmentSlot> EquipmentSlotRef;
	TWeakObjectPtr<UInventoryManagerComponent> InventoryManagerRef;
};
