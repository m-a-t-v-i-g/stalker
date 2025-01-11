// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDStatsWidget.h"

#include "CharacterArmorComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/ProgressBar.h"

void UHUDStatsWidget::SetupStatsWidget(UAbilitySystemComponent* AbilityComp, UCharacterArmorComponent* ArmorComp)
{
	AbilityComponentRef = AbilityComp;
	if (AbilityComponentRef.IsValid())
	{
		HealthAttribute = Cast<UHealthAttributeSet>(AbilityComponentRef->GetAttributeSet(UHealthAttributeSet::StaticClass()));
		if (!HealthAttribute.IsValid())
		{
			return;
		}
	
		auto& MaxHealthAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
			HealthAttribute->GetMaxHealthAttribute());
		auto& HealthAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
			HealthAttribute->GetHealthAttribute());

		MaxHealthAttrDelegate.AddUObject(this, &UHUDStatsWidget::OnMaxHealthUpdated);
		HealthAttrDelegate.AddUObject(this, &UHUDStatsWidget::OnHealthUpdated);
	
		ForceUpdateHealthBar();
	}

	ArmorComponentRef = ArmorComp;
	if (ArmorComponentRef.IsValid())
	{
		ArmorComponentRef->OnTotalArmorDataChangedDelegate.AddUObject(
			this, &UHUDStatsWidget::OnTotalArmorEnduranceUpdated);

		ForceUpdateArmorBar();
	}
}

void UHUDStatsWidget::ClearStatsWidget()
{
	HealthBar->SetPercent(0.0f);
	ArmorBar->SetPercent(0.0f);

	if (AbilityComponentRef.IsValid())
	{
		if (HealthAttribute.IsValid())
		{
			auto& MaxHealthAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
				HealthAttribute->GetMaxHealthAttribute());
			auto& HealthAttrDelegate = AbilityComponentRef->GetGameplayAttributeValueChangeDelegate(
				HealthAttribute->GetHealthAttribute());

			MaxHealthAttrDelegate.RemoveAll(this);
			HealthAttrDelegate.RemoveAll(this);
		}
	}

	if (ArmorComponentRef.IsValid())
	{
		ArmorComponentRef->OnTotalArmorDataChangedDelegate.RemoveAll(this);
	}
	
	AbilityComponentRef.Reset();
	HealthAttribute.Reset();
	ArmorComponentRef.Reset();
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
