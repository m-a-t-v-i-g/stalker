// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayAbility_MagazineReload.h"
#include "AbilitySystemComponent.h"
#include "InventoryComponent.h"
#include "StalkerGameplayTags.h"
#include "Ammo/AmmoObject.h"
#include "Weapons/WeaponObject.h"

UStalkerGameplayAbility_MagazineReload::UStalkerGameplayAbility_MagazineReload(
	const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

UWeaponObject* UStalkerGameplayAbility_MagazineReload::GetWeaponObject() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UWeaponObject>(Spec->SourceObject.Get());
	}
	return nullptr;
}

UInventoryComponent* UStalkerGameplayAbility_MagazineReload::GetInventoryComponent() const
{
	if (APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		return AvatarPawn->GetComponentByClass<UInventoryComponent>();
	}
	return nullptr;
}

void UStalkerGameplayAbility_MagazineReload::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                           const FGameplayAbilityActorInfo* ActorInfo,
                                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                                           const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	OnReloadCompleteCallbackDelegateHandle = AbilityComponent->AbilityReplicatedEventDelegate(
		EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnReloadCompleteCallback);

	OnReloadCancelledCallbackDelegateHandle = AbilityComponent->AbilityReplicatedEventDelegate(
		EAbilityGenericReplicatedEvent::GenericCancel, CurrentSpecHandle,
		CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnReloadCancelledCallback);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TryReloadWeapon();
}

void UStalkerGameplayAbility_MagazineReload::EndAbility(const FGameplayAbilitySpecHandle Handle,
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
		
		UWeaponObject* WeaponObject = GetWeaponObject();
		check(WeaponObject);
		
		UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(AbilityComponent);

		if (CurrentActorInfo->IsNetAuthority())
		{
			WeaponObject->CancelAllActionsDelegate.RemoveAll(this);
		}
		
		AbilityComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericConfirm,
		                                                 CurrentSpecHandle,
		                                                 CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnReloadCompleteCallbackDelegateHandle);

		AbilityComponent->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel,
		                                                 CurrentSpecHandle,
		                                                 CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnReloadCancelledCallbackDelegateHandle);

		if (!bWasCancelled)
		{
			AbilityComponent->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm,
			                                                CurrentSpecHandle,
			                                                CurrentActivationInfo.GetActivationPredictionKey());
		}
		else
		{
			AbilityComponent->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel,
			                                                CurrentSpecHandle,
			                                                CurrentActivationInfo.GetActivationPredictionKey());
		}
		
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UStalkerGameplayAbility_MagazineReload::TryReloadWeapon()
{
	UWeaponObject* WeaponObject = GetWeaponObject();
	check(WeaponObject);

	if (WeaponObject->IsMagFull() || !HasAmmoForReload())
	{
		K2_CancelAbility();
		return;
	}

	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);
	
	AbilityComponent->AddLooseGameplayTag(FStalkerGameplayTags::BlockAbilityTag_NoFiring);
	
	if (CurrentActorInfo->IsNetAuthority())
	{
		WeaponObject->CancelAllActionsDelegate.AddUObject(this, &UStalkerGameplayAbility_MagazineReload::CancelReloadWeapon);

		UAmmoObject* Ammo = GetAmmoForReload(WeaponObject->GetCurrentAmmoClass());
		check(Ammo);

		int AmmoCount = Ammo->GetAmount();
		int RequiredCount = WeaponObject->CalculateRequiredAmmoCount();
		int AvailableCount = FMath::Min(RequiredCount, AmmoCount);

		ReloadingData.AmmoObject = Ammo;
		ReloadingData.AmmoCount = AvailableCount;
		SetReloadTimer();
	}

	GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, "Reloading...");
}

void UStalkerGameplayAbility_MagazineReload::CancelReloadWeapon()
{
	OnReloadCancelledCallback();
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
}

void UStalkerGameplayAbility_MagazineReload::SetReloadTimer()
{
	UWeaponObject* WeaponObject = GetWeaponObject();
	check(WeaponObject);
	
	FTimerDelegate ReloadDelegate;
	ReloadDelegate.BindLambda([&, this]
	{
		OnReloadCompleteCallback();
	});

	float ReloadTime = WeaponObject->GetReloadTime();
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, ReloadDelegate, ReloadTime, false);
}

bool UStalkerGameplayAbility_MagazineReload::HasAmmoForReload() const
{
	if (UWeaponObject* WeaponObject = GetWeaponObject())
	{
		if (UAmmoObject* Ammo = GetAmmoForReload(WeaponObject->GetCurrentAmmoClass()))
		{
			return Ammo->GetAmount() > 0;
		}
	}
	return false;
}

UAmmoObject* UStalkerGameplayAbility_MagazineReload::GetAmmoForReload(const UAmmoDefinition* DesiredAmmo) const
{
	if (UWeaponObject* WeaponObject = GetWeaponObject())
	{
		UAmmoObject* ResultAmmo;

		if (DesiredAmmo)
		{
			ResultAmmo = Cast<UAmmoObject>(GetInventoryComponent()->FindItemByDefinition(DesiredAmmo));
			if (ResultAmmo)
			{
				return ResultAmmo;
			}
		}

		for (const UAmmoDefinition* AmmoClass : WeaponObject->GetAmmoClasses())
		{
			ResultAmmo = Cast<UAmmoObject>(GetInventoryComponent()->FindItemByDefinition(AmmoClass));
			if (ResultAmmo)
			{
				return ResultAmmo;
			}
		}
	}
	return nullptr;
}

void UStalkerGameplayAbility_MagazineReload::OnReloadCompleteCallback()
{
	GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, "Reloaded!");
	
	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	AbilityComponent->RemoveLooseGameplayTag(FStalkerGameplayTags::BlockAbilityTag_NoFiring);

	if (!CurrentActorInfo->IsLocallyControlled() && CurrentActorInfo->IsNetAuthority())
	{
		AbilityComponent->ClientSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle,
		                                           CurrentActivationInfo.GetActivationPredictionKey());
	}
	
	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		if (CurrentActorInfo->IsNetAuthority())
		{
			GetInventoryComponent()->SubtractOrRemoveItem(ReloadingData.AmmoObject, ReloadingData.AmmoCount);
			GetWeaponObject()->IncreaseAmmo(ReloadingData.AmmoObject, ReloadingData.AmmoCount);
		}
	}
	
	AbilityComponent->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, CurrentSpecHandle,
	                                                CurrentActivationInfo.GetActivationPredictionKey());
	K2_EndAbility();
}

void UStalkerGameplayAbility_MagazineReload::OnReloadCancelledCallback()
{
	GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Red, "Reload cancelled...");
	
	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* AbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(AbilityComponent);

	AbilityComponent->RemoveLooseGameplayTag(FStalkerGameplayTags::BlockAbilityTag_NoFiring);

	if (!CurrentActorInfo->IsLocallyControlled() && CurrentActorInfo->IsNetAuthority())
	{
		AbilityComponent->ClientSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, CurrentSpecHandle,
												   CurrentActivationInfo.GetActivationPredictionKey());
	}
	
	K2_CancelAbility();
}
