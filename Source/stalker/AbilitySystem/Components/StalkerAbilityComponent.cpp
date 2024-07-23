// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerAbilityComponent.h"
#include "AbilitySystem/Attributes/OrganicAttributeSet.h"

UStalkerAbilityComponent::UStalkerAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	OrganicAttributeSet = CreateDefaultSubobject<UOrganicAttributeSet>(TEXT("OrganicAttributeSet"));
	
	AddAttributeSetSubobject<UOrganicAttributeSet>(OrganicAttributeSet);
	
	FAttributeDefaults ActionAttribute;
	ActionAttribute.Attributes = OrganicAttributeSet.GetClass();
	DefaultStartingData.Add(ActionAttribute);
}

void UStalkerAbilityComponent::InitAbilitySystem(AController* InController, AActor* InActor)
{
	InitAbilityActorInfo(InController, InActor);
}
