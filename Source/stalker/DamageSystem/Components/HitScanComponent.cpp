// Fill out your copyright notice in the Description page of Project Settings.

#include "HitScanComponent.h"

UHitScanComponent::UHitScanComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHitScanComponent::AddBoneToScanMap(FName BoneName, FGameplayTag Tag)
{
	HitScanMap.Add(BoneName, Tag);
}

void UHitScanComponent::HitOwnerPart(const FGameplayTag& DamageTag, const FHitResult& HitResult, float DamageValue)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Hit part: %s"), *HitResult.BoneName.ToString()));
	
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
