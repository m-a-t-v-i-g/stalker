// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_RangedWeaponAiming.h"

#include "CharacterWeaponComponent.h"
#include "StalkerCharacter.h"

UStalkerGameplayAbility_RangedWeaponAiming::UStalkerGameplayAbility_RangedWeaponAiming(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
}

UCharacterWeaponComponent* UStalkerGameplayAbility_RangedWeaponAiming::GetWeaponComponent() const
{
	if (AStalkerCharacter* StalkerPawn = Cast<AStalkerCharacter>(GetAvatarActorFromActorInfo()))
	{
		return StalkerPawn->GetWeaponComponent();
	}
	return nullptr;
}

void UStalkerGameplayAbility_RangedWeaponAiming::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                           const FGameplayAbilityActorInfo* ActorInfo,
                                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UCharacterWeaponComponent* WeaponComponent = GetWeaponComponent())
	{
		WeaponComponent->StartAiming();
	}
}

void UStalkerGameplayAbility_RangedWeaponAiming::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo,
																  ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}
		
		if (UCharacterWeaponComponent* WeaponComponent = GetWeaponComponent())
		{
			WeaponComponent->StopAiming();
		}
		
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UStalkerGameplayAbility_RangedWeaponAiming::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	K2_EndAbility();
}
