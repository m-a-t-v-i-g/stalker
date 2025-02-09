// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DamageSystemCore.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FApplyDamageData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;

	UPROPERTY()
	TWeakObjectPtr<AActor> SourceActor;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

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

	FApplyDamageData()
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
	
	void SetSourceActor(AActor* SourceAct)
	{
		SourceActor = SourceAct;
	}
	
	void SetTargetActor(AActor* TargetAct)
	{
		TargetActor = TargetAct;
	}
	
	void SetInstigator(AActor* InInstigator, AActor* InDamageCauser)
	{
		Instigator = InInstigator;
		DamageCauser = InDamageCauser;
	}

	void SetDamageInfo(UClass* DamageType, float DmgValue)
	{
		DamageTypeClass = DamageType;
		DamageValue = DmgValue;
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
	static void TakeDamage(const FApplyDamageData& DamageData);

	static bool TakeDamageASCtoASC(const FApplyDamageData& DamageData, FActiveGameplayEffectHandle& OutDamageEffectHandle);
	static bool TakeDamageActorToASC(const FApplyDamageData& DamageData, FActiveGameplayEffectHandle& OutDamageEffectHandle);
	static void TakeDamageActorToActor(const FApplyDamageData& DamageData, float& OutResultDamage);
	
	static FApplyDamageData GenerateDamageData(AActor* SourceActor, AActor* TargetActor, AActor* DamageCauser,
	                                           UClass* DamageType, float DamageValue,
	                                           const FHitResult& SweepResult, const UObject* SourceObject);
};
