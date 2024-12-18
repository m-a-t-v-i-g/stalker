// Fill out your copyright notice in the Description page of Project Settings.

#include "ResistanceAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UResistanceAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, BulletResistance, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, BlastResistance,	COND_OwnerOnly, REPNOTIFY_Always);
}

void UResistanceAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetBulletResistanceAttribute())
	{
		SetBulletResistance(Data.EvaluatedData.Magnitude);
	}
}

void UResistanceAttributeSet::OnRep_BulletResistance(const FGameplayAttributeData& OldBulletResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResistanceAttributeSet, BulletResistance, OldBulletResistance);
}

void UResistanceAttributeSet::OnRep_BlastResistance(const FGameplayAttributeData& OldBlastResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UResistanceAttributeSet, BlastResistance, OldBlastResistance);
}
