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
	TObjectPtr<class UWidgetSwitcher> TabSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDWidget> HUD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UPlayerManagerWidget> Manager;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> PDA;
	
private:
	TWeakObjectPtr<UStalkerAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<class UCharacterInventoryComponent> OwnInventoryComponent;
	
public:
	void InitializeMainWidget(UStalkerAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp);
	
	void ToggleTab(EActivateTab ActivateTab);
};
