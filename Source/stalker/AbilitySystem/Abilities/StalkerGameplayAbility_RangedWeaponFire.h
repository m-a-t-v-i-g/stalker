// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StalkerGameplayAbility.h"
#include "StalkerGameplayAbility_RangedWeaponFire.generated.h"

class UWeaponObject;

UENUM(BlueprintType)
enum class EWeaponTargetingSource : uint8
{
	// From the player's camera towards camera focus
	CameraTowardsFocus,
	// From the pawn's center, in the pawn's orientation
	PawnForward,
	// From the pawn's center, oriented towards camera focus
	PawnTowardsFocus,
	// From the weapon's muzzle or location, in the pawn's orientation
	WeaponForward,
	// From the weapon's muzzle or location, towards camera focus
	WeaponTowardsFocus,
	// Custom blueprint-specified source location
	Custom
};

UCLASS()
class STALKER_API UStalkerGameplayAbility_RangedWeaponFire : public UStalkerGameplayAbility
{
	GENERATED_BODY()

public:
	UStalkerGameplayAbility_RangedWeaponFire(const FObjectInitializer& ObjectInitializer);
	
	UWeaponObject* GetWeaponObject() const;

protected:
	struct FRangedWeaponFiringInput
	{
		FVector StartTrace;

		FVector EndAim;

		FVector AimDir;

		UWeaponObject* WeaponObject = nullptr;

		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	
	void StartWeaponTargeting();
	
	void PerformLocalTargeting(TArray<FHitResult>& OutHits);
	
	void TraceBulletsInCartridge(const FRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits);

	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius,
	                               bool bIsSimulated, TArray<FHitResult>& OutHits) const;
	
	FHitResult BulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated,
	                       TArray<FHitResult>& OutHitResults) const;

	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

	FTransform GetTargetingTransform(APawn* SourcePawn, EWeaponTargetingSource Source) const;
	
private:
	FTimerHandle EndAbilityTimer;
	
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	void SpawnBullets(const FGameplayAbilityTargetDataHandle& InData);
};
