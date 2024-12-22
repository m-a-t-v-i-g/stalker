// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDStatsWidget.h"

#include "CharacterArmorComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/ProgressBar.h"

void UHUDStatsWidget::SetupStatsWidget(UOrganicAbilityComponent* AbilityComp, UCharacterArmorComponent* ArmorComp)
{
	ArmorComponentRef = ArmorComp;
	if (!ArmorComponentRef.IsValid())
	{
		return;
	}
	
	HealthAttribute = Cast<UHealthAttributeSet>(AbilityComp->GetAttributeSet(UHealthAttributeSet::StaticClass()));
	if (!HealthAttribute.IsValid())
	{
		return;
	}
	
	auto& MaxHealthAttrDelegate = AbilityComp->GetGameplayAttributeValueChangeDelegate(HealthAttribute->GetMaxHealthAttribute());
	auto& HealthAttrDelegate = AbilityComp->GetGameplayAttributeValueChangeDelegate(HealthAttribute->GetHealthAttribute());

	MaxHealthAttrDelegate.AddUObject(this, &UHUDStatsWidget::OnMaxHealthUpdated);
	HealthAttrDelegate.AddUObject(this, &UHUDStatsWidget::OnHealthUpdated);

	ArmorComponentRef->OnTotalArmorDataChangedDelegate.AddUObject(this, &UHUDStatsWidget::OnTotalArmorEnduranceUpdated);

	ForceUpdateHealthBar();
	ForceUpdateArmorBar();
}

void UHUDStatsWidget::OnMaxHealthUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	float HealthValue = HealthAttribute->GetHealth();
	HealthBar->SetPercent(HealthValue / AttributeChangeData.NewValue);
}

void UHUDStatsWidget::OnHealthUpdated(const FOnAttributeChangeData& AttributeChangeData)
{
	float MaxHealthValue = HealthAttribute->GetMaxHealth();
	HealthBar->SetPercent(AttributeChangeData.NewValue / MaxHealthValue);
}

void UHUDStatsWidget::OnTotalArmorEnduranceUpdated(const FTotalArmorData& ArmorEnduranceData)
{
	int ArmorPartsNum = ArmorEnduranceData.ArmorPartsNum;
	float FinalArmorPercent = 0.0f;
	if (ArmorPartsNum > 0)
	{
		FinalArmorPercent = ArmorEnduranceData.TotalArmorEndurance / ArmorPartsNum / 100.0f;
	}
	
	ArmorBar->SetPercent(FinalArmorPercent);
}

void UHUDStatsWidget::ForceUpdateHealthBar()
{
	FOnAttributeChangeData Data;
	Data.NewValue = HealthAttribute->GetHealth();
	
	OnHealthUpdated(Data);
}

void UHUDStatsWidget::ForceUpdateArmorBar()
{
	OnTotalArmorEnduranceUpdated(ArmorComponentRef->GetTotalArmorEndurance());
}
