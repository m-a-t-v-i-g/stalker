// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_Jump.generated.h"

UCLASS()
class STALKER_API UStalkerGameplayAbility_Jump : public UStalkerGameplayAbility
{
	GENERATED_BODY()

public:
	UStalkerGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	UFUNCTION()
	void CharacterJumpStart();

	UFUNCTION()
	void CharacterJumpStop();
};
