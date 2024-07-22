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
	TObjectPtr<class UHUDWidget> HUD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UPlayerManagerWidget> Manager;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> PDA;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;
	
public:
	void InitializeHUD(class UStalkerAbilityComponent* AbilityComponent);
	void InitializeManager(class UItemsContainerComponent* ItemsContainerComponent);

	void ToggleTab(EActivateTab ActivateTab);
};
