// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDStatsWidget.generated.h"

struct FOnAttributeChangeData;

class UOrganicAttributeSet;
class UOrganicAbilityComponent;

UCLASS()
class STALKER_API UHUDStatsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ArmorBar;

private:
	FDelegateHandle MaxHealthDelHandle;
	FDelegateHandle HealthDelHandle;

	TWeakObjectPtr<const UOrganicAttributeSet> Attributes;

public:
	void SetupStatsWidget(UOrganicAbilityComponent* AbilityComp);

protected:
	void OnMaxHealthUpdated(const FOnAttributeChangeData& AttributeChangeData);
	void OnHealthUpdated(const FOnAttributeChangeData& AttributeChangeData);

	void ForceUpdateHealthBar();
};
