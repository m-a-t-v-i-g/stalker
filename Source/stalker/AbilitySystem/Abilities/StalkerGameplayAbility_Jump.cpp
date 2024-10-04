// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_Jump.h"
#include "BaseOrganic.h"

UStalkerGameplayAbility_Jump::UStalkerGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UStalkerGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	CharacterJumpStart();
}

void UStalkerGameplayAbility_Jump::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo)
{
	CharacterJumpStop();
	K2_EndAbility();
}

void UStalkerGameplayAbility_Jump::CharacterJumpStart()
{
	ABaseOrganic* Organic = CastChecked<ABaseOrganic>(CurrentActorInfo->AvatarActor.Get());
	Organic->StartAction1();
}

void UStalkerGameplayAbility_Jump::CharacterJumpStop()
{
	ABaseOrganic* Organic = CastChecked<ABaseOrganic>(CurrentActorInfo->AvatarActor.Get());
	Organic->StopAction1();
}
