// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_WeaponReload.h"
#include "AbilitySystemComponent.h"
#include "Weapons/WeaponObject.h"

UStalkerGameplayAbility_WeaponReload::UStalkerGameplayAbility_WeaponReload(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

UWeaponObject* UStalkerGameplayAbility_WeaponReload::GetWeaponObject() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UWeaponObject>(Spec->SourceObject.Get());
	}
	return nullptr;
}

void UStalkerGameplayAbility_WeaponReload::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                           const FGameplayAbilityActorInfo* ActorInfo,
                                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                                           const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	OnReloadCompleteCallbackDelegateHandle = AbilityComponent->AbilityReplicatedEventDelegate(
		EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnReloadCompleteCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (HasAuthority(&CurrentActivationInfo))
	{
		OnReloadCompleteCallback();
	}
}

void UStalkerGameplayAbility_WeaponReload::EndAbility(const FGameplayAbilitySpecHandle Handle,
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

		UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(AbilityComponent);

		AbilityComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericConfirm,
		                                                 CurrentSpecHandle,
		                                                 CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnReloadCompleteCallbackDelegateHandle);
		AbilityComponent->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UStalkerGameplayAbility_WeaponReload::OnReloadCompleteCallback()
{
	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	if (HasAuthority(&CurrentActivationInfo))
	{
		AbilityComponent->ClientSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	}
	
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{

	}
	else
	{
		K2_EndAbility();
	}
	
	AbilityComponent->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}
