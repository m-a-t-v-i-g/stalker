// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/GameHUD.h"
#include "GameWidget.generated.h"

class UItemsContainer;

UCLASS()
class STALKER_API UGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeGameWidget(UInventoryManagerComponent* InventoryManagerComp);

	void ConnectCharacterPart(const FCharacterHUDInitData& HUDInitData);
	void DisconnectCharacterPart();
	
	void SetupAndOpenOwnInventory();
	void CloseOwnInventory();
	
	void OpenEmptyTab();
	void OpenLootingTab(UInventoryComponent* InventoryToLoot);

	void OpenHUDTab();
	void ActivateSlotManager();

	FORCEINLINE UInventoryManagerWidget* GetInventoryManagerWidget() const { return InventoryManagerWidget; }
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDWidget> HUD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> SlotManager;

private:
	TWeakObjectPtr<UInventoryManagerComponent> OwnInventoryManager;
	TObjectPtr<UInventoryManagerWidget> InventoryManagerWidget;
	
	TWeakObjectPtr<UAbilitySystemComponent> CharAbilityComponent;
	TWeakObjectPtr<UInventoryComponent> CharInventoryComponent;
	TWeakObjectPtr<UEquipmentComponent> CharEquipmentComponent;
};
