// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility.h"

UStalkerGameplayAbility::UStalkerGameplayAbility(const FObjectInitializer& ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
}

AController* UStalkerGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

void UStalkerGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UStalkerGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}
