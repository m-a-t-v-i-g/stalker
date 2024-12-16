// Fill out your copyright notice in the Description page of Project Settings.

#include "ExecCalculation_BulletDamage.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HealthAttributeSet.h"

void UExecCalculation_BulletDamage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	if (!TargetASC)
	{
		return;
	}
	
	float FinalDamage = ExecutionParams.GetOwningSpecForPreExecuteMod()->GetSetByCallerMagnitude(DamageTag, false);
	if (FinalDamage > 0.0f)
	{
		bool bAttributeFound;
		float DamageResistanceValue = TargetASC->GetGameplayAttributeValue(ResistanceAttribute, bAttributeFound);

		if (bAttributeFound)
		{
			FinalDamage *= 1 - DamageResistanceValue / 100.0f;
		}
		
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive,
			                               FinalDamage));
	}
}
