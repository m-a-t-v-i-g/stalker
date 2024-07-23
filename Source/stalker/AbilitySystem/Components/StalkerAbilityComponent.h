// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StalkerAbilityComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = "Stalker")
class STALKER_API UStalkerAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UStalkerAbilityComponent();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Attributes")
	TObjectPtr<class UOrganicAttributeSet> OrganicAttributeSet;

	void InitAbilitySystem(AController* InController, AActor* InActor);
};
