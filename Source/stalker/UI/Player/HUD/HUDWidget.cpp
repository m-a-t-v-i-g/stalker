// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDWidget.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Components/HUDStatsWidget.h"

void UHUDWidget::InitializeHUD(UOrganicAbilityComponent* AbilityComp)
{
	OwnAbilityComponent = AbilityComp;

	check(OwnAbilityComponent.IsValid());

	StatsWidget->SetupStatsWidget(OwnAbilityComponent.Get());
}
