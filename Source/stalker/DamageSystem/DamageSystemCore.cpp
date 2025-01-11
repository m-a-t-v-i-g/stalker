// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageSystemCore.h"
#include "AbilitySystemComponent.h"
#include "DamageType_Base.h"
#include "GameplayCueNotifyTypes.h"
#include "Components/HitScanComponent.h"
#include "Engine/DamageEvents.h"

/* TODO: out ActiveGameplayEffectHandle and OutDamageValue */
void UDamageSystemCore::TakeDamage(const FApplyDamageData& DamageData)
{
	if (DamageData.TargetActor.IsValid())
	{
		if (DamageData.TargetActor->GetComponentByClass<UAbilitySystemComponent>())
		{
			if (DamageData.SourceASC.IsValid())
			{
				FActiveGameplayEffectHandle GameplayEffectHandle;
				TakeDamageASCtoASC(DamageData, GameplayEffectHandle);
			}
			else
			{
				FActiveGameplayEffectHandle GameplayEffectHandle;
				TakeDamageActorToASC(DamageData, GameplayEffectHandle);
			}
		}
		else
		{
			float OutDamageValue;
			TakeDamageActorToActor(DamageData, OutDamageValue);	
		}
	}
}

bool UDamageSystemCore::TakeDamageASCtoASC(const FApplyDamageData& DamageData, FActiveGameplayEffectHandle& OutDamageEffectHandle)
{
	if (!DamageData.SourceASC.IsValid() || !DamageData.TargetASC.IsValid())
	{
		return false;
	}
	
	if (auto DamageCDO = DamageData.DamageTypeClass->GetDefaultObject<UDamageType_Base>())
	{
		if (UClass* DamageEffectClass = DamageCDO->GetDamageEffect())
		{
			auto Context = FGameplayEffectContextHandle(new FGameplayEffectContext);
			if (Context.IsValid())
			{
				Context.AddSourceObject(DamageData.SourceObject);
				Context.AddHitResult(DamageData.HitResult);
				Context.AddInstigator(DamageData.Instigator.Get(), DamageData.DamageCauser.Get());

				auto DamageEffect = DamageEffectClass->GetDefaultObject<UGameplayEffect>();
				auto DamageSpec = FGameplayEffectSpec(DamageEffect, Context, 1.0f);
				DamageSpec.SetSetByCallerMagnitude(DamageCDO->GetDamageTag(), DamageData.DamageValue);

				OutDamageEffectHandle = DamageData.SourceASC->ApplyGameplayEffectSpecToTarget(
					DamageSpec, DamageData.TargetASC.Get());

				if (AActor* TargetAbilityCompOwner = DamageData.TargetASC->GetOwner())
				{
					if (auto HitScanComponent = TargetAbilityCompOwner->GetComponentByClass<UHitScanComponent>())
					{
						HitScanComponent->HitOwnerPart(DamageCDO->GetDamageTag(), DamageData.HitResult,
						                               DamageData.DamageValue);
					}
				}
				return true;
			}
		}
	}
	return false;
}

bool UDamageSystemCore::TakeDamageActorToASC(const FApplyDamageData& DamageData, FActiveGameplayEffectHandle& OutDamageEffectHandle)
{
	if (!DamageData.TargetASC.IsValid())
	{
		return false;
	}
	
	if (auto DamageCDO = DamageData.DamageTypeClass->GetDefaultObject<UDamageType_Base>())
	{
		if (UClass* DamageEffectClass = DamageCDO->GetDamageEffect())
		{
			auto Context = FGameplayEffectContextHandle(new FGameplayEffectContext);
			if (Context.IsValid())
			{
				Context.AddSourceObject(DamageData.SourceObject);
				Context.AddHitResult(DamageData.HitResult);
				Context.AddInstigator(DamageData.Instigator.Get(), DamageData.DamageCauser.Get());
				
				auto DamageEffect = DamageEffectClass->GetDefaultObject<UGameplayEffect>();
				auto DamageSpec = FGameplayEffectSpec(DamageEffect, Context, 1.0f);
				DamageSpec.SetSetByCallerMagnitude(DamageCDO->GetDamageTag(), DamageData.DamageValue);

				OutDamageEffectHandle = DamageData.TargetASC->ApplyGameplayEffectSpecToSelf(DamageSpec);

				if (AActor* TargetAbilityCompOwner = DamageData.TargetASC->GetOwner())
				{
					if (auto HitScanComponent = TargetAbilityCompOwner->GetComponentByClass<UHitScanComponent>())
					{
						HitScanComponent->HitOwnerPart(DamageCDO->GetDamageTag(), DamageData.HitResult,
													   DamageData.DamageValue);
					}
				}
				return true;
			}
		}
	}
	return false;
}

void UDamageSystemCore::TakeDamageActorToActor(const FApplyDamageData& DamageData, float& OutResultDamage)
{
	if (!DamageData.TargetActor.IsValid() || !DamageData.Instigator.IsValid())
	{
		return;
	}

	OutResultDamage = DamageData.TargetActor->TakeDamage(DamageData.DamageValue,
	                                                     FDamageEvent(DamageData.DamageTypeClass),
	                                                     DamageData.Instigator->GetInstigatorController(),
	                                                     DamageData.DamageCauser.Get());

	if (auto DamageCDO = DamageData.DamageTypeClass->GetDefaultObject<UDamageType_Base>())
	{
		if (UHitScanComponent* HitScanComponent = DamageData.TargetActor->GetComponentByClass<UHitScanComponent>())
		{
			HitScanComponent->HitOwnerPart(DamageCDO->GetDamageTag(), DamageData.HitResult, DamageData.DamageValue);
		}
	}
}

FApplyDamageData UDamageSystemCore::GenerateDamageData(AActor* SourceActor, AActor* TargetActor, AActor* DamageCauser,
                                                       UClass* DamageType, float DamageValue,
                                                       const FHitResult& SweepResult, const UObject* SourceObject)
{
	FApplyDamageData GenerateDamageData;

	if (SourceActor)
	{
		GenerateDamageData.SetSourceActor(SourceActor);
		
		if (UAbilitySystemComponent* SourceASC = SourceActor->GetComponentByClass<UAbilitySystemComponent>())
		{
			GenerateDamageData.SetSourceASC(SourceASC);
		}
	}
	
	if (TargetActor)
	{
		GenerateDamageData.SetTargetActor(TargetActor);
		
		if (UAbilitySystemComponent* TargetASC = TargetActor->GetComponentByClass<UAbilitySystemComponent>())
		{
			GenerateDamageData.SetTargetASC(TargetASC);
		}
	}

	GenerateDamageData.SetInstigator(SourceActor, DamageCauser);
	GenerateDamageData.SetDamageInfo(DamageType, DamageValue);
	GenerateDamageData.SetHitResult(SweepResult);
	GenerateDamageData.SetSourceObject(SourceObject);
	
	return GenerateDamageData;
}
