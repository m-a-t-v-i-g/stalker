// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StalkerAbilityComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = "Stalker")
class STALKER_API UStalkerAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

protected:
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

public:
	UStalkerAbilityComponent();
	
	void InitAbilitySystem(AController* InController, AActor* InActor);
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float DeltaSeconds);
};
