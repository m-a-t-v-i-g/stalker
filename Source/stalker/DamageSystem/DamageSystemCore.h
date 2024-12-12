// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageSystemCore.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class STALKER_API UDamageSystemCore : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool TakeDamageASCtoASC(AActor* Instigator, AActor* DamageCauser, float Damage,
	                               const FHitResult& HitResult, UAbilitySystemComponent* SourceASC,
	                               UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> DamageEffectClass);
	static bool TakeDamageActorToASC(AActor* Instigator, AActor* DamageCauser, float Damage,
	                                 const FHitResult& HitResult, UAbilitySystemComponent* TargetASC,
	                                 TSubclassOf<UGameplayEffect> DamageEffectClass);
	static bool TakeDamageActorToActor(AActor* Instigator, AActor* DamageCauser, AActor* TargetActor, float Damage,
	                                   TSubclassOf<UDamageType> DamageClass);
};
