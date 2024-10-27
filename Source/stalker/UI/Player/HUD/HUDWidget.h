// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UOrganicAbilityComponent;

UCLASS(DisplayName = "HUD Widget")
class STALKER_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDStatsWidget> StatsWidget;

private:
	TWeakObjectPtr<UOrganicAbilityComponent> OwnAbilityComponent;
	
public:
	void InitializeHUD(UOrganicAbilityComponent* AbilityComp);
};
