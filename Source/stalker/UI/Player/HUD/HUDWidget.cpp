// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/HUDStatsWidget.h"

void UHUDWidget::InitializeHUD(UStalkerAbilityComponent* AbilityComp)
{
	OwnAbilityComponent = AbilityComp;

	check(OwnAbilityComponent.IsValid());

	StatsWidget->SetupStatsWidget(OwnAbilityComponent.Get());
}
