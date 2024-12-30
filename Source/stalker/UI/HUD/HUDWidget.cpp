// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "AbilitySystemComponent.h"
#include "CharacterArmorComponent.h"
#include "HUDInteractionWidget.h"
#include "HUDStatsWidget.h"
#include "PawnInteractionComponent.h"
#include "StalkerPlayerController.h"

void UHUDWidget::InitializeHUDWidget()
{
	
}

void UHUDWidget::ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo)
{
	OwnAbilityComponent = HUDInitInfo.AbilitySystemComponent;
	OwnArmorComponent = HUDInitInfo.ArmorComponent;
	OwnInteractionComponent = HUDInitInfo.InteractionComponent;

	StatsWidget->SetupStatsWidget(OwnAbilityComponent.Get(), OwnArmorComponent.Get());
	InteractionWidget->SetupInteractionWidget(OwnInteractionComponent.Get());
}

void UHUDWidget::DisconnectCharacterHUD()
{
	OwnAbilityComponent.Reset();
	OwnArmorComponent.Reset();
	OwnInteractionComponent.Reset();
}
