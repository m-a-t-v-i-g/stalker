// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDStatsWidget.h"
#include "AbilitySystem/Attributes/OrganicAttributeSet.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/ProgressBar.h"

void UHUDStatsWidget::SetupStatsWidget(UStalkerAbilityComponent* AbilityComp)
{
	Attributes = Cast<UOrganicAttributeSet>(AbilityComp->GetAttributeSet(UOrganicAttributeSet::StaticClass()));
	if (!Attributes.IsValid()) return;
	
	auto& MaxHealthAttrDelegate = AbilityComp->GetGameplayAttributeValueChangeDelegate(Attributes->GetHealthAttribute());
	auto& HealthAttrDelegate = AbilityComp->GetGameplayAttributeValueChangeDelegate(Attributes->GetMaxHealthAttribute());

	TDelegate<void(const FOnAttributeChangeData&)> HealthDelegate;
	TDelegate<void(const FOnAttributeChangeData&)> MaxHealthDelegate;

	MaxHealthDelHandle = MaxHealthAttrDelegate.Add(MaxHealthDelegate);
	HealthDelHandle = HealthAttrDelegate.Add(HealthDelegate);

	HealthDelegate.BindUObject(this, &UHUDStatsWidget::OnHealthUpdated);
	MaxHealthDelegate.BindUObject(this, &UHUDStatsWidget::OnMaxHealthUpdated);

	ForceUpdateHealthBar();
}

void UHUDStatsWidget::OnMaxHealthUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	float CurrentValue = Attributes->GetHealth();
	HealthBar->SetPercent(CurrentValue / AttributeChangeData.NewValue);
}

void UHUDStatsWidget::OnHealthUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	float MaxValue = Attributes->GetMaxHealth();
	HealthBar->SetPercent(AttributeChangeData.NewValue / MaxValue);
}

void UHUDStatsWidget::ForceUpdateHealthBar()
{
	float MaxValue = Attributes->GetMaxHealth();
	float CurrentValue = Attributes->GetHealth();
	HealthBar->SetPercent(CurrentValue / MaxValue);
}
