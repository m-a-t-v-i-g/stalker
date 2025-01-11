// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

struct FCharacterHUDInitData;
class UAbilitySystemComponent;
class UCharacterWeaponComponent;
class UCharacterArmorComponent;
class UPawnInteractionComponent;

UCLASS(DisplayName = "HUD Widget")
class STALKER_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeHUDWidget();
	
	void ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo);
	void DisconnectCharacterHUD();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDBaseReticleWidget> ReticleWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDStatsWidget> StatsWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHUDInteractionWidget> InteractionWidget;

private:
	TWeakObjectPtr<UAbilitySystemComponent> OwnAbilityComponent;
	TWeakObjectPtr<UCharacterWeaponComponent> OwnWeaponComponent;
	TWeakObjectPtr<UCharacterArmorComponent> OwnArmorComponent;
	TWeakObjectPtr<UPawnInteractionComponent> OwnInteractionComponent;
};
