// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

UENUM()
enum class EActivateTab : uint8
{
	HUD,
	Inventory,
	PDA
};

UCLASS()
class STALKER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void PostInitializeComponents() override;
	
	UPROPERTY(EditDefaultsOnly, DisplayName = "HUD Widget Class", Category = "HUD")
	TSubclassOf<class UPlayerMainWidget> HUDWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UInteractiveItemWidget> InteractiveItemWidgetClass; 

	TObjectPtr<UPlayerMainWidget> HUDWidget;

private:
	EActivateTab ActiveTab = EActivateTab::HUD;
	
public:
	static UClass* StaticInteractiveItemWidgetClass;
	static float TileSize;
	
	void InitializePlayerInventory(class UItemsContainerComponent* ItemsContainerComponent);

	void ToggleTab(EActivateTab Tab);
};
