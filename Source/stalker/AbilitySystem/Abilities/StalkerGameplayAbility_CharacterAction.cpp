// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_CharacterAction.h"

UStalkerGameplayAbility_CharacterAction::UStalkerGameplayAbility_CharacterAction(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UStalkerGameplayAbility_CharacterAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	CharacterActionStart();
}

void UStalkerGameplayAbility_CharacterAction::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo)
{
	CharacterActionStop();
	K2_EndAbility();
}

void UStalkerGameplayAbility_CharacterAction::CharacterActionStart_Implementation()
{
}

void UStalkerGameplayAbility_CharacterAction::CharacterActionStop_Implementation()
{
}
