// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageSystemCore.h"
#include "AbilitySystemComponent.h"
#include "StalkerGameplayTags.h"
#include "Engine/DamageEvents.h"

bool UDamageSystemCore::TakeDamageASCtoASC(AActor* Instigator, AActor* DamageCauser, float Damage,
                                           const FHitResult& HitResult, UAbilitySystemComponent* SourceASC,
                                           UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> DamageEffectClass)
{
	check(Instigator);
	check(DamageCauser);
	check(SourceASC);
	check(TargetASC);
	
	if (DamageEffectClass)
	{
		auto DamageEffect = NewObject<UGameplayEffect>(SourceASC, DamageEffectClass,
		                                               FName(DamageCauser->GetName() + "_" + DamageEffectClass->GetName()));
		if (DamageEffect)
		{
			auto Context = FGameplayEffectContextHandle(new FGameplayEffectContext);
			if (Context.IsValid())
			{
				Context.AddHitResult(HitResult);
				Context.AddInstigator(Instigator, DamageCauser);
				
				auto DamageSpec = FGameplayEffectSpec(DamageEffect, Context, 1.0f);
				DamageSpec.SetSetByCallerMagnitude(FStalkerGameplayTags::EffectTag_Damage, Damage);
				SourceASC->ApplyGameplayEffectSpecToTarget(DamageSpec, TargetASC);
				return true;
			}
		}
	}
	return false;
}

bool UDamageSystemCore::TakeDamageActorToASC(AActor* Instigator, AActor* DamageCauser, float Damage,
                                             const FHitResult& HitResult, UAbilitySystemComponent* TargetASC,
                                             TSubclassOf<UGameplayEffect> DamageEffectClass)
{
	check(Instigator);
	check(DamageCauser);
	check(TargetASC);
	
	if (DamageEffectClass)
	{
		auto DamageEffect = NewObject<UGameplayEffect>(TargetASC, DamageEffectClass,
		                                               FName(DamageCauser->GetName() + "_" + DamageEffectClass->GetName()));
		if (DamageEffect)
		{
			auto Context = FGameplayEffectContextHandle(new FGameplayEffectContext);
			if (Context.IsValid())
			{
				Context.AddHitResult(HitResult);
				Context.AddInstigator(Instigator, DamageCauser);
				
				auto DamageSpec = FGameplayEffectSpec(DamageEffect, Context, 1.0f);
				DamageSpec.SetSetByCallerMagnitude(FStalkerGameplayTags::EffectTag_Damage, Damage);
				TargetASC->ApplyGameplayEffectSpecToSelf(DamageSpec);
				return true;
			}
		}
	}
	return false;
}

bool UDamageSystemCore::TakeDamageActorToActor(AActor* Instigator, AActor* DamageCauser, AActor* TargetActor, float Damage,
                                               TSubclassOf<UDamageType> DamageClass)
{
	check(Instigator);
	check(DamageCauser);
	check(TargetActor);
	
	float ResultDamage = TargetActor->TakeDamage(Damage, FDamageEvent(DamageClass),
	                                             Instigator->GetInstigatorController(), DamageCauser);
	if (ResultDamage > 0.0f)
	{
		return true;
	}
	return false;
}
