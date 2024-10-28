// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_Crouch.h"
#include "Character/BaseCharacter.h"

UStalkerGameplayAbility_Crouch::UStalkerGameplayAbility_Crouch(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UStalkerGameplayAbility_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo,
                                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	CharacterCrouchStart();
}

void UStalkerGameplayAbility_Crouch::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo)
{
	CharacterCrouchStop();
	K2_EndAbility();
}

void UStalkerGameplayAbility_Crouch::CharacterCrouchStart()
{
	if (ABaseCharacter* Organic = CastChecked<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		Organic->StartAction2();
	}
}

void UStalkerGameplayAbility_Crouch::CharacterCrouchStop()
{
	if (ABaseCharacter* Organic = CastChecked<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		Organic->StopAction2();
	}
}
