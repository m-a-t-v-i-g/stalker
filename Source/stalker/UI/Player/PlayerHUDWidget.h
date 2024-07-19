// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

UCLASS()
class STALKER_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> HUDSwitcher;
	
public:
	void InitializeHUDWidget();

	void InitializePlayerInventory(class UItemsContainerComponent* ItemsContainerComponent);

	void ToggleInventory(bool bEnable);
};
