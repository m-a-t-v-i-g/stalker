// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

struct FPlayerInitInfo;
class UOrganicAbilityComponent;
class UPawnInteractionComponent;

UCLASS(DisplayName = "HUD Widget")
class STALKER_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDStatsWidget> StatsWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDInteractionWidget> InteractionWidget;

private:
	TWeakObjectPtr<UOrganicAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<UPawnInteractionComponent> OwnInteractionComponent;
	
public:
	void InitializeHUD(const FPlayerInitInfo& CharacterInitInfo);
};
