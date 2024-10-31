// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerHUD.h"
#include "PlayerMainWidget.generated.h"

class UItemsContainer;

UCLASS()
class STALKER_API UPlayerMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OpenInventory();
	void CloseInventory();
	
	void StartLooting(UItemsContainerComponent* LootItemsContainer);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDWidget> HUD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> SlotManager;
	
private:
	TWeakObjectPtr<UOrganicAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<UCharacterInventoryComponent> OwnInventoryComponent;

	TObjectPtr<class UPlayerManagerWidget> InventoryWidget;
	
public:
	void InitializeMainWidget(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp);
	
	void ToggleTab(EHUDTab ActivateTab);

};
