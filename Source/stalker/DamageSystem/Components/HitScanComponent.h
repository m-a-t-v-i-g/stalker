// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "HitScanComponent.generated.h"

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UHitScanComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitScanComponent();

	TMulticastDelegate<void(const FGameplayTag&, const FGameplayTag&, const FHitResult&, float)> OnOwnerDamagedDelegate;

	void HitOwnerPart(const FGameplayTag& DamageTag, const FHitResult& HitResult, float DamageValue);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Hit Scan")
	TMap<FName, FGameplayTag> HitScanMap;
	
	virtual void OnOwnerHit(const FGameplayTag& DamageTag, const FGameplayTag& PartTag, const FHitResult& HitResult, float DamageValue);
};
