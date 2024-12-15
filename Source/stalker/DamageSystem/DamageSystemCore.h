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
	static bool TakeDamageASCtoASC(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, AActor* Instigator,
	                               AActor* DamageCauser, TSubclassOf<UGameplayEffect> DamageEffectClass,
	                               float Damage, const FHitResult& HitResult, const UObject* SourceObject);
	static bool TakeDamageActorToASC(AActor* Instigator, AActor* DamageCauser, float Damage,
	                                 const FHitResult& HitResult, UAbilitySystemComponent* TargetASC,
	                                 TSubclassOf<UGameplayEffect> DamageEffectClass);
	static bool TakeDamageActorToActor(AActor* Instigator, AActor* DamageCauser, AActor* TargetActor, float Damage,
	                                   TSubclassOf<UDamageType> DamageClass);
};
