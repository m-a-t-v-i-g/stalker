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
	UPROPERTY(EditDefaultsOnly, Category = "HUD|Inputs")
	TObjectPtr<const UInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|Inputs")
	TArray<FInputConfigData> InputConfigData;
	
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UGameWidget> GameWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UInventoryManagerWidget> InventoryWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UItemWidget> ItemWidgetClass; 

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UGameWidget> GameWidgetRef;

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UUserWidget> MainMenuWidgetRef;

	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInput(UInputComponent* PlayerInputComp);

	void CreateAndShowGameWidget();
	void CreateAndShowMainMenuWidget();
	
	void ToggleTab(EHUDTab Tab, bool bForce = false);
	
	void OpenHUD();
	
	void SetupAndOpenOwnInventory();
	void CloseOwnInventory();

	void SetupAndOpenLootingTab(UInventoryComponent* InventoryToLoot);

	void ClearAll();
	
	void SetGameOnlyMode();
	void SetGameAndUIMode(const UWidget* WidgetToFocus, bool bHideCursorDuringCapture);
	void SetUIOnlyMode(const UWidget* WidgetToFocus, bool bHideCursorDuringCapture);

	virtual void ToggleInventory();
	virtual void ToggleSlot(uint8 SlotIndex);

	UFUNCTION()
	void IA_Escape(const FInputActionInstance& InputAction);
	
private:
	TWeakObjectPtr<AStalkerCharacter> CharacterRef;
	TWeakObjectPtr<UPawnInteractionComponent> InteractionComponentRef;
	
	EHUDTab ActiveTab = EHUDTab::HUD;
};
