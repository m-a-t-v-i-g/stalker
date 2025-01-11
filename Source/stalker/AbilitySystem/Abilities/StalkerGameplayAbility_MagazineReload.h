// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_MagazineReload.generated.h"

class UInventoryComponent;
class UWeaponObject;
class UAmmoDefinition;
class UAmmoObject;

USTRUCT()
struct FReloadingData
{
	GENERATED_USTRUCT_BODY()

	TObjectPtr<UAmmoObject> AmmoObject;
	uint16 AmmoCount = 0;

	FReloadingData()
	{
	}
	
	void Clear()
	{
		AmmoObject = nullptr;
		AmmoCount = 0;
	}

	bool IsValid() const
	{
		return AmmoObject != nullptr && AmmoCount > 0;
	}
};

UCLASS()
class STALKER_API UStalkerGameplayAbility_MagazineReload : public UStalkerGameplayAbility
{
	GENERATED_BODY()
	
public:
	UStalkerGameplayAbility_MagazineReload(const FObjectInitializer& ObjectInitializer);
	
	UWeaponObject* GetWeaponObject() const;
	UInventoryComponent* GetInventoryComponent() const;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	void TryReloadWeapon();
	void CancelReloadWeapon();
	
	void SetReloadTimer();
	
	bool HasAmmoForReload() const;
	UAmmoObject* GetAmmoForReload(const UAmmoDefinition* DesiredAmmo) const;
	
private:
	FReloadingData ReloadingData;
	FTimerHandle ReloadTimerHandle;
	
	FDelegateHandle OnReloadCompleteCallbackDelegateHandle;
	FDelegateHandle OnReloadCancelledCallbackDelegateHandle;

	void OnReloadCompleteCallback();
	void OnReloadCancelledCallback();
};
