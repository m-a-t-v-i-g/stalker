// Fill out your copyright notice in the Description page of Project Settings.

#include "OrganicAttributeSet.h"
#include "Net/UnrealNetwork.h"

void UOrganicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UOrganicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOrganicAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

void UOrganicAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOrganicAttributeSet, MaxHealth, OldMaxHealth);
}

void UOrganicAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOrganicAttributeSet, Health, OldHealth);
}
