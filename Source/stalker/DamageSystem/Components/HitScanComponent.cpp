// Fill out your copyright notice in the Description page of Project Settings.

#include "HitScanComponent.h"

UHitScanComponent::UHitScanComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHitScanComponent::HitOwnerPart(const FGameplayTag& DamageTag, const FHitResult& HitResult, float DamageValue)
{
	FGameplayTag* ImpactedPartTag = HitScanMap.Find(HitResult.BoneName);
	if (ImpactedPartTag && ImpactedPartTag->IsValid())
	{
		OnOwnerHit(DamageTag, *ImpactedPartTag, HitResult, DamageValue);
	}
}

void UHitScanComponent::OnOwnerHit(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
                                   const FHitResult& HitResult, float DamageValue)
{
	OnOwnerDamagedDelegate.Broadcast(DamageTag, PartTag, HitResult, DamageValue);
}
