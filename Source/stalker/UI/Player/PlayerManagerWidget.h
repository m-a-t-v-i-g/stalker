// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerManagerWidget.generated.h"

UENUM(BlueprintType)
enum class EPlayerManagerTab : uint8
{
	Inventory,
	Looting,
	Upgrading
};

class UStalkerAbilityComponent;
class UInventoryComponent;
class UCharacterInventoryComponent;
class UItemObject;

UCLASS()
class STALKER_API UPlayerManagerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> Looting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> Upgrading;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> Inventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCharacterEquipmentWidget> Equipment;

private:
	TWeakObjectPtr<UStalkerAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<UCharacterInventoryComponent> OwnInventoryComponent;
	
	TWeakObjectPtr<UInventoryComponent> LootingInventoryComponent;

	EPlayerManagerTab ActiveTab;
	
public:
	void InitializeManager(UStalkerAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp);

	void StartLooting(UInventoryComponent* LootInventory);
	void StartUpgrading();

	void ClearTabs();

	void ActivateTab(EPlayerManagerTab TabToActivate);
	
protected:
	void OnOwnInventoryItemDoubleClick(UItemObject* ItemObject);
	void OnOwnEquippedItemDoubleClick(const FString& SlotName);
	void OnLootingItemDoubleClick(UItemObject* ItemObject);
};
