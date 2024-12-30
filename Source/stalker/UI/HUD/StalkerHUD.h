// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerPlayerController.h"
#include "GameFramework/HUD.h"
#include "StalkerHUD.generated.h"

class UItemsContainer;
class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UInventoryComponent;

UENUM()
enum class EHUDTab : uint8
{
	HUD,
	Inventory
};

UCLASS()
class STALKER_API AStalkerHUD : public AHUD
{
	GENERATED_BODY()

public:
	static UClass* StaticInventoryWidgetClass;
	static UClass* StaticItemWidgetClass;
	
	static float TileSize;
	
	void InitializeHUD(UInventoryManagerComponent* InventoryManagerComp);

	void ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo);
	void ClearCharacterHUD();

	void OnOpenInventory();
	void OnLootInventory(UInventoryComponent* TargetInventory);
	
	void ToggleTab(EHUDTab Tab, bool bForce = false);
	
	void SetupAndOpenOwnInventory();
	void CloseOwnInventory();

	void StartLooting(UInventoryComponent* InventoryToLoot);

	void OpenHUD();
	
	void ClearAll();
	
protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UMainWidget> MainWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UInventoryManagerWidget> InventoryWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UItemWidget> ItemWidgetClass; 

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UMainWidget> MainWidget;

	void SetGameOnlyMode();
	void SetGameAndUIMode();
	
private:
	TWeakObjectPtr<AStalkerCharacter> CharacterRef;
	
	EHUDTab ActiveTab = EHUDTab::HUD;
};
