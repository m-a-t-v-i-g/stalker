// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_Crouch.generated.h"

UCLASS()
class STALKER_API UStalkerGameplayAbility_Crouch : public UStalkerGameplayAbility
{
	GENERATED_BODY()
		
public:
	UStalkerGameplayAbility_Crouch(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	UFUNCTION()
	void CharacterCrouchStart();

	UFUNCTION()
	void CharacterCrouchStop();
};
