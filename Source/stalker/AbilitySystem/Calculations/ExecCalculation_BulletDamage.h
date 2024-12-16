// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalculation_BulletDamage.generated.h"

UCLASS()
class STALKER_API UExecCalculation_BulletDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	                                    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Calculation")
	FGameplayTag DamageTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Calculation")
	FGameplayAttribute ResistanceAttribute;
};
