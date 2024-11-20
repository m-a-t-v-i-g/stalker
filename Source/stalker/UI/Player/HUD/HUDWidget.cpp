// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "Components/HUDInteractionWidget.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/HUDStatsWidget.h"
#include "Components/PawnInteractionComponent.h"
#include "Player/StalkerPlayerController.h"

void UHUDWidget::InitializeHUD(const FPlayerInitInfo& CharacterInitInfo)
{
	OwnAbilityComponent = CharacterInitInfo.AbilitySystemComponent;
	OwnInteractionComponent = CharacterInitInfo.InteractionComponent;

	check(OwnAbilityComponent.IsValid() && OwnInteractionComponent.IsValid());

	StatsWidget->SetupStatsWidget(OwnAbilityComponent.Get());
	InteractionWidget->SetupInteractionWidget(OwnInteractionComponent.Get());
}
