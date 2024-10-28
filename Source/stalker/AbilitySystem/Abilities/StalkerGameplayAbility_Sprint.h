// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_Sprint.generated.h"

UCLASS()
class STALKER_API UStalkerGameplayAbility_Sprint : public UStalkerGameplayAbility
{
	GENERATED_BODY()

public:
	UStalkerGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	UFUNCTION()
	void CharacterSprintStart();

	UFUNCTION()
	void CharacterSprintStop();
};
