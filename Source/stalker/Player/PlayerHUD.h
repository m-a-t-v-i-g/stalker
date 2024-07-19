// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

UCLASS()
class STALKER_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, DisplayName = "HUD Widget Class", Category = "HUD")
	TSubclassOf<class UPlayerHUDWidget> HUDWidgetClass; 

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UInteractiveItemWidget> InteractiveItemWidgetClass; 

	TObjectPtr<UPlayerHUDWidget> HUDWidget;
	
	virtual void PostInitializeComponents() override;

public:
	static UClass* StaticInteractiveItemWidgetClass;
	static float TileSize;
	
	void InitializePlayerInventory(class UItemsContainerComponent* ItemsContainerComponent);
};
