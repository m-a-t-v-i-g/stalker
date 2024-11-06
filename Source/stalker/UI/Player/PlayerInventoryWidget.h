// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInventoryWidget.generated.h"

class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UInventoryComponent;
class UItemObject;

UENUM()
enum class EPlayerInventoryTab : uint8
{
	Inventory,
	Looting,
	Upgrading
};

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
	
	TWeakObjectPtr<UInventoryComponent> LootingInventory;

	EPlayerInventoryTab ActiveTab;
	
public:
	void OpenInventory(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp);
	void CloseInventory();

	void StartLooting(UInventoryComponent* LootItemsContainer);
	void StartUpgrading();

	void ClearTabs();

	void ActivateTab(EPlayerInventoryTab TabToActivate);
	
protected:
	void OnOwnInventoryItemDoubleClick(UItemObject* ItemObject);
	void OnOwnEquippedItemDoubleClick(const FString& SlotName);
	void OnLootingItemDoubleClick(UItemObject* ItemObject);
};
