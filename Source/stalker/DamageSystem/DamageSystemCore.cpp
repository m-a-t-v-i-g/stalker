// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageSystemCore.h"
#include "AbilitySystemComponent.h"
#include "DamageType_Base.h"
#include "StalkerGameplayTags.h"
#include "Components/HitScanComponent.h"
#include "Engine/DamageEvents.h"

bool UDamageSystemCore::TakeDamageASCtoASC(const FDamageDataASCtoASC& DamageData, FActiveGameplayEffectHandle& OutDamageEffectHandle)
{
	check(DamageData.SourceASC.IsValid());
	check(DamageData.TargetASC.IsValid());
	
	if (UDamageType_Base* DamageCDO = DamageData.DamageTypeClass->GetDefaultObject<UDamageType_Base>())
	{
		if (UClass* DamageEffectClass = DamageCDO->GetDamageEffect())
		{
			auto DamageEffect = NewObject<UGameplayEffect>(DamageData.SourceASC.Get(), DamageEffectClass,
			                                               FName(DamageData.DamageCauser->GetName() + "_" +
				                                               DamageEffectClass->GetName()));
			if (DamageEffect)
			{
				auto Context = FGameplayEffectContextHandle(new FGameplayEffectContext);
				if (Context.IsValid())
				{
					Context.AddSourceObject(DamageData.SourceObject);
					Context.AddHitResult(DamageData.HitResult);
					Context.AddInstigator(DamageData.Instigator.Get(), DamageData.DamageCauser.Get());
				
					auto DamageSpec = FGameplayEffectSpec(DamageEffect, Context, 1.0f);
					DamageSpec.SetSetByCallerMagnitude(DamageCDO->GetDamageTag(), DamageData.DamageValue);
				
					OutDamageEffectHandle = DamageData.SourceASC->ApplyGameplayEffectSpecToTarget(
						DamageSpec, DamageData.TargetASC.Get());
				
					if (AActor* TargetAbilityCompOwner = DamageData.TargetASC->GetOwner())
					{
						if (UHitScanComponent* HitScanComponent = TargetAbilityCompOwner->GetComponentByClass<UHitScanComponent>())
						{
							HitScanComponent->HitOwnerPart(DamageCDO->GetDamageTag(), DamageData.HitResult);
						}
					}
					return true;
				}
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
				//DamageSpec.SetSetByCallerMagnitude(FStalkerGameplayTags::EffectTag_Damage, Damage);
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
		if (UHitScanComponent* HitScanComponent = TargetActor->GetComponentByClass<UHitScanComponent>())
		{
			//HitScanComponent->HitOwnerPart(DamageCDO->GetDamageTag(), DamageData.HitResult);
		}
		return true;
	}
	return false;
}
