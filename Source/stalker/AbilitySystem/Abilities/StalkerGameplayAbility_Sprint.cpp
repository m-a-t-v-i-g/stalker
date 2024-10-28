// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_Sprint.h"
#include "Character/BaseCharacter.h"

UStalkerGameplayAbility_Sprint::UStalkerGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UStalkerGameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo,
                                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	CharacterSprintStart();
}

void UStalkerGameplayAbility_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo)
{
	CharacterSprintStop();
	K2_EndAbility();
}

void UStalkerGameplayAbility_Sprint::CharacterSprintStart()
{
	if (ABaseCharacter* Organic = CastChecked<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		Organic->StartAction3();
	}
}

void UStalkerGameplayAbility_Sprint::CharacterSprintStop()
{
	if (ABaseCharacter* Organic = CastChecked<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		Organic->StopAction3();
	}
}
