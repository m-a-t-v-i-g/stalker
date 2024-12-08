// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/StalkerHUD.h"
#include "MainWidget.generated.h"

class UItemsContainer;

UCLASS()
class STALKER_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeMainWidget(const FPlayerInitInfo& CharacterInitInfo);
	
	void SetupOwnInventory();
	void CloseOwnInventory();
	
	void OpenEmptyTab();
	void OpenLootingTab(UInventoryComponent* InventoryToLoot);

	void OpenHUDTab();
	void OpenInventoryTab();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDWidget> HUD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> SlotManager;
	
private:
	TWeakObjectPtr<UOrganicAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<UInventoryComponent> OwnInventoryComponent;
	TWeakObjectPtr<UEquipmentComponent> OwnEquipmentComponent;
	TWeakObjectPtr<UInventoryManagerComponent> OwnInventoryManager;

	TObjectPtr<UInventoryManagerWidget> InventoryManagerWidget;
};
