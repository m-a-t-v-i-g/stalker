// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/StalkerPlayerController.h"
#include "GameFramework/HUD.h"
#include "StalkerHUD.generated.h"

class UItemsContainer;
class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UInventoryComponent;

UCLASS()
class STALKER_API AStalkerHUD : public AHUD
{
	GENERATED_BODY()

public:
	static UClass* StaticInventoryWidgetClass;
	static UClass* StaticItemWidgetClass;
	
	static float TileSize;
	
	void InitializePlayerHUD(const FCharacterInitInfo& CharacterInitInfo);

	void ToggleTab(EHUDTab Tab, bool bForce = false);
	
	void OpenInventory();
	void CloseInventory();

	void StartLooting(UInventoryComponent* InventoryToLoot);
	
protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UPlayerMainWidget> MainWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UPlayerInventoryWidget> InventoryWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UItemWidget> ItemWidgetClass; 

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UPlayerMainWidget> MainWidget;

private:
	EHUDTab ActiveTab = EHUDTab::HUD;
};
