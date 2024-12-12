// Fill out your copyright notice in the Description page of Project Settings.

#include "ExecCalculation_BlastDamage.h"
#include "AbilitySystemComponent.h"
#include "StalkerGameplayTags.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/ResistanceAttributeSet.h"

void UExecCalculation_BlastDamage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	if (!TargetASC)
	{
		return;
	}
	
	float FinalDamage = ExecutionParams.GetOwningSpecForPreExecuteMod()->GetSetByCallerMagnitude(
		FStalkerGameplayTags::EffectTag_Damage, false);
	
	bool bBlastResFound;
	float BlastResistanceValue = TargetASC->GetGameplayAttributeValue(
		UResistanceAttributeSet::GetBlastResistanceAttribute(), bBlastResFound);

	if (bBlastResFound)
	{
		FinalDamage *= 1 - BlastResistanceValue / 100.0f;
	}

	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive,
										   FinalDamage));
	}
}
