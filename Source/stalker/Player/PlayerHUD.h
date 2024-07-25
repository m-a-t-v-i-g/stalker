// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UStalkerAbilityComponent;
class UCharacterInventoryComponent;

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
	TSubclassOf<class UPlayerMainWidget> MainWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UInteractiveItemWidget> InteractiveItemWidgetClass; 

	UPROPERTY(EditInstanceOnly, Category = "HUD")
	TObjectPtr<UPlayerMainWidget> MainWidget;

private:
	EActivateTab ActiveTab = EActivateTab::HUD;
	
public:
	static UClass* StaticInteractiveItemWidgetClass;
	static float TileSize;
	
	void InitializePlayerHUD(UStalkerAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp);

	void ToggleTab(EActivateTab Tab);
};
