// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_RangedWeaponAiming.generated.h"

class UCharacterWeaponComponent;

UCLASS()
class STALKER_API UStalkerGameplayAbility_RangedWeaponAiming : public UStalkerGameplayAbility
{
	GENERATED_BODY()

public:
	UStalkerGameplayAbility_RangedWeaponAiming(const FObjectInitializer& ObjectInitializer);

	UCharacterWeaponComponent* GetWeaponComponent() const;
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) override;
};
