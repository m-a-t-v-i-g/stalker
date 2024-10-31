// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerPlayerController.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UItemsContainer;
class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UItemsContainerComponent;

UCLASS()
class STALKER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	void OpenInventory();
	void CloseInventory();
	
protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UPlayerMainWidget> MainWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UPlayerManagerWidget> InventoryWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UItemWidget> ItemWidgetClass; 

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UPlayerMainWidget> MainWidget;

private:
	EHUDTab ActiveTab = EHUDTab::HUD;
	
public:
	static UClass* StaticInventoryWidgetClass;
	static UClass* StaticItemWidgetClass;
	
	static float TileSize;
	
	void InitializePlayerHUD(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp);

	void ToggleTab(EHUDTab& Tab, bool bForce = false);
	
	void StartLooting(UItemsContainerComponent* LootItemsContainer);
};
