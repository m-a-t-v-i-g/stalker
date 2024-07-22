// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerHUD.h"
#include "PlayerMainWidget.generated.h"

UCLASS()
class STALKER_API UPlayerMainWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> HUD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UPlayerInventoryWidget> Inventory;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> PDA;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;
	
public:
	void InitializeHUDWidget();

	void InitializePlayerInventory(class UItemsContainerComponent* ItemsContainerComponent);

	void ToggleTab(EActivateTab ActivateTab);
};
