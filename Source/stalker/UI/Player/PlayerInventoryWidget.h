// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerHUD.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInventoryWidget.generated.h"

class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UInventoryComponent;
class UItemObject;

UCLASS()
class STALKER_API UPlayerInventoryWidget : public UUserWidget
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
	TWeakObjectPtr<UCharacterInventoryComponent> OwnInventoryComponent;
	TWeakObjectPtr<UPlayerInventoryManagerComponent> OwnInventoryManager;
	TWeakObjectPtr<UInventoryComponent> LootingInventory;

	EPlayerInventoryTab ActiveTab;
	
public:
	void OpenInventory(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp, UPlayerInventoryManagerComponent* PlayerInventoryManager);
	void CloseInventory();

	void OpenEmpty();
	void OpenLooting(UInventoryComponent* LootItemsContainer);
	void OpenUpgrading();

	void ClearTabs();

	void ActivateTab(EPlayerInventoryTab TabToActivate);
	
protected:
	void OnOwnInventoryItemDoubleClick(UItemObject* ItemObject);
	void OnOwnEquippedItemDoubleClick(const FString& SlotName);
	void OnLootingInventoryItemDoubleClick(UItemObject* ItemObject);
};
