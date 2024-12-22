// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDStatsWidget.generated.h"

struct FTotalArmorData;
struct FOnAttributeChangeData;
class UOrganicAbilityComponent;
class UCharacterArmorComponent;
class UHealthAttributeSet;

UCLASS()
class STALKER_API UHUDStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupStatsWidget(UOrganicAbilityComponent* AbilityComp, UCharacterArmorComponent* ArmorComp);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ArmorBar;

	void OnMaxHealthUpdated(const FOnAttributeChangeData& AttributeChangeData);
	void OnHealthUpdated(const FOnAttributeChangeData& AttributeChangeData);

	void OnTotalArmorEnduranceUpdated(const FTotalArmorData& ArmorEnduranceData);
	
	void ForceUpdateHealthBar();
	void ForceUpdateArmorBar();

private:
	FDelegateHandle MaxHealthDelHandle;
	FDelegateHandle HealthDelHandle;

	TWeakObjectPtr<const UHealthAttributeSet> HealthAttribute;
	TWeakObjectPtr<UCharacterArmorComponent> ArmorComponentRef;
};
