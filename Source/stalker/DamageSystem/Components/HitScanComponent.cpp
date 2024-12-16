// Fill out your copyright notice in the Description page of Project Settings.

#include "HitScanComponent.h"

UHitScanComponent::UHitScanComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHitScanComponent::HitOwnerPart(const FGameplayTag& DamageTag, const FHitResult& HitResult)
{
	FGameplayTag* ImpactedPartTag = HitScanMap.Find(HitResult.BoneName);
	if (ImpactedPartTag && ImpactedPartTag->IsValid())
	{
		OnOwnerHit(DamageTag, *ImpactedPartTag, HitResult);
	}
}

void UHitScanComponent::OnOwnerHit(const FGameplayTag& DamageTag, const FGameplayTag& PartTag, const FHitResult& HitResult)
{
	OnOwnerDamagedDelegate.Broadcast(DamageTag, PartTag, HitResult);
}
