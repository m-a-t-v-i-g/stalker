// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerHUD.h"
#include "Blueprint/UserWidget.h"
#include "InventoryManagerWidget.generated.h"

class UOrganicAbilityComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UItemObject;

UCLASS()
class STALKER_API UInventoryManagerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> Inventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCharacterEquipmentWidget> Equipment;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> Looting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> Upgrading;

private:
	TWeakObjectPtr<UOrganicAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<UInventoryComponent> OwnInventoryComponent;
	TWeakObjectPtr<UEquipmentComponent> OwnEquipmentComponent;
	TWeakObjectPtr<UInventoryManagerComponent> OwnInventoryManager;
	TWeakObjectPtr<UInventoryComponent> LootingInventory;

	EPlayerInventoryTab ActiveTab;

public:
	void OpenInventory(UOrganicAbilityComponent* AbilityComp, UInventoryComponent* InventoryComp,
	                   UEquipmentComponent* EquipmentComp, UInventoryManagerComponent* InventoryManager);
	void CloseInventory();

	void OpenEmpty();
	void OpenLooting(UInventoryComponent* LootItemsContainer);
	void OpenUpgrading();

	void ClearTabs();

	void ActivateTab(EPlayerInventoryTab TabToActivate);
	
protected:
	void OnOwnInventoryItemDoubleClick(UItemObject* ItemObject);
	void OnOwnEquippedItemDoubleClick(UEquipmentSlot* EquipmentSlot);
	void OnLootingInventoryItemDoubleClick(UItemObject* ItemObject);
};
