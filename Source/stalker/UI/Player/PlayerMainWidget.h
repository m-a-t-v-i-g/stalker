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
	TWeakObjectPtr<UOrganicAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<class UCharacterInventoryComponent> OwnInventoryComponent;
	
public:
	void InitializeMainWidget(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp,
							 UItemsContainer* ItemsContainer);
	
	void ToggleTab(EHUDTab ActivateTab);

	void StartLooting(UItemsContainerComponent* LootItemsContainer);
};
