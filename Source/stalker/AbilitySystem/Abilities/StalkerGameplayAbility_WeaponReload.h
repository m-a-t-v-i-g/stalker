// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_WeaponReload.generated.h"

class UWeaponObject;

UCLASS()
class STALKER_API UStalkerGameplayAbility_WeaponReload : public UStalkerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UStalkerGameplayAbility_WeaponReload(const FObjectInitializer& ObjectInitializer);
	
	UWeaponObject* GetWeaponObject() const;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	
private:
	FDelegateHandle OnReloadCompleteCallbackDelegateHandle;

	void OnReloadCompleteCallback();
};
