// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UStalkerAbilityComponent;

UCLASS(DisplayName = "HUD Widget")
class STALKER_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDStatsWidget> StatsWidget;

private:
	TWeakObjectPtr<UStalkerAbilityComponent> OwnAbilityComponent;
	
public:
	void InitializeHUD(UStalkerAbilityComponent* AbilityComp);
};
