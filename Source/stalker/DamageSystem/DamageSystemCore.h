// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DamageSystemCore.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FDamageDataASCtoASC
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;

	UPROPERTY()
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY()
	TWeakObjectPtr<AActor> DamageCauser;

	UPROPERTY()
	UClass* DamageTypeClass = nullptr;
	
	UPROPERTY()
	TObjectPtr<const UObject> SourceObject;
	
	FHitResult HitResult;
	
	float DamageValue = 0.0f;

	FDamageDataASCtoASC()
	{
	}

	void SetSourceASC(UAbilitySystemComponent* InSourceASC)
	{
		SourceASC = InSourceASC;
	}
	
	void SetTargetASC(UAbilitySystemComponent* InTargetASC)
	{
		TargetASC = InTargetASC;
	}
	
	void SetInstigator(AActor* InInstigator, AActor* InDamageCauser)
	{
		Instigator = InInstigator;
		DamageCauser = InDamageCauser;
	}

	void SetDamageInfo(UClass* DamageType, float Value)
	{
		DamageTypeClass = DamageType;
		DamageValue = Value;
	}
	
	void SetHitResult(const FHitResult& Result)
	{
		HitResult = Result;
	}
	
	void SetSourceObject(const UObject* InSourceObject)
	{
		SourceObject = InSourceObject;
	}
};

UCLASS()
class STALKER_API UDamageSystemCore : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool TakeDamageASCtoASC(const FDamageDataASCtoASC& DamageData, FActiveGameplayEffectHandle& OutDamageEffectHandle);
	static bool TakeDamageActorToASC(AActor* Instigator, AActor* DamageCauser, float Damage,
	                                 const FHitResult& HitResult, UAbilitySystemComponent* TargetASC,
	                                 TSubclassOf<UGameplayEffect> DamageEffectClass);
	static bool TakeDamageActorToActor(AActor* Instigator, AActor* DamageCauser, AActor* TargetActor, float Damage,
	                                   TSubclassOf<UDamageType> DamageClass);
};
