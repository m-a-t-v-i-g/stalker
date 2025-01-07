// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerPlayerController.h"
#include "GameFramework/HUD.h"
#include "GameHUD.generated.h"

struct FInputConfigData;
class UWidget;
class UOrganicAbilityComponent;
class UInventoryComponent;
class UItemsContainer;

UENUM()
enum class EHUDTab : uint8
{
	HUD,
	Inventory
};

UCLASS()
class STALKER_API AGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	static UClass* StaticInventoryWidgetClass;
	static UClass* StaticItemWidgetClass;
	
	static float TileSize;

	void InitializeHUD(UInventoryManagerComponent* InventoryManagerComp, UInputComponent* PlayerInputComp);

	void ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo);
	void ClearCharacterHUD();

	void OpenOwnInventory();
	void LootOtherInventory(UInventoryComponent* OtherInventory);
	
	void OnInteraction(AActor* TargetActor);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TObjectPtr<const UInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TArray<FInputConfigData> InputConfigData;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UMainWidget> MainWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UInventoryManagerWidget> InventoryWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UItemWidget> ItemWidgetClass; 

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UMainWidget> MainWidget;

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UUserWidget> MainMenuWidget;

	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComp);
	virtual void SetupCharacterInput(UInputComponent* CharInputComp);

	void CreateAndShowGameWidget();
	void CreateAndShowMainMenuWidget();
	
	void ToggleTab(EHUDTab Tab, bool bForce = false);
	
	void OpenHUD();
	
	void SetupAndOpenOwnInventory();
	void CloseOwnInventory();

	void SetupAndOpenLootingTab(UInventoryComponent* InventoryToLoot);

	void ClearAll();
	
	void SetGameOnlyMode();
	void SetGameAndUIMode(const UWidget* WidgetToFocus, bool bShowCursorDuringCapture);
	void SetUIOnlyMode(const UWidget* WidgetToFocus, bool bShowCursorDuringCapture);

	virtual void ToggleInventory(const FInputActionInstance& InputAction);

	UFUNCTION()
	void IA_Escape(const FInputActionInstance& InputAction);
	
private:
	TWeakObjectPtr<AStalkerCharacter> CharacterRef;
	TWeakObjectPtr<UPawnInteractionComponent> InteractionComponentRef;
	
	EHUDTab ActiveTab = EHUDTab::HUD;
};
