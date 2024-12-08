// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "HUDInteractionWidget.h"
#include "HUDStatsWidget.h"
#include "PawnInteractionComponent.h"
#include "StalkerPlayerController.h"
#include "Components/OrganicAbilityComponent.h"

void UHUDWidget::InitializeHUD(const FPlayerInitInfo& CharacterInitInfo)
{
	OwnAbilityComponent = CharacterInitInfo.AbilitySystemComponent;
	OwnInteractionComponent = CharacterInitInfo.InteractionComponent;

	check(OwnAbilityComponent.IsValid() && OwnInteractionComponent.IsValid());

	StatsWidget->SetupStatsWidget(OwnAbilityComponent.Get());
	InteractionWidget->SetupInteractionWidget(OwnInteractionComponent.Get());
}
