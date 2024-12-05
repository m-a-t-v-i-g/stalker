// Fill out your copyright notice in the Description page of Project Settings.

#include "BulletBase.h"
#include "AbilitySystemComponent.h"
#include "Ammo/AmmoObject.h"
#include "DamageSystem/DamageSystemCore.h"
#include "Weapons/WeaponObject.h"

ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABulletBase::SetupBullet(UWeaponObject* Weapon, UAmmoObject* Ammo)
{
	check(Weapon);
	check(Ammo);
	
	BulletData.Owner = GetOwner();
	BulletData.Instigator = GetInstigator();

	if (BulletData.Instigator.IsValid())
	{
		BulletData.OwnerAbilityComponent = BulletData.Instigator->GetComponentByClass<UAbilitySystemComponent>();
	}

	BulletData.DamageType = Ammo->GetDamageType();
	BulletData.BaseDamage = Ammo->GetDamageData().BaseDamage;
	BulletData.DamageEffects = Ammo->GetDamageEffects();
}

void ABulletBase::HitLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto OtherAbilityComp = OtherActor->GetComponentByClass<UAbilitySystemComponent>())
		{
			if (BulletData.DamageEffects.IsEmpty())
			{
				return;
			}

			if (BulletData.OwnerAbilityComponent.IsValid())
			{
				for (UClass* EffectClass : BulletData.DamageEffects)
				{
					UDamageSystemCore::TakeDamageASCtoASC(BulletData.Instigator.Get(), this, SweepResult,
					                                      BulletData.OwnerAbilityComponent.Get(), OtherAbilityComp,
					                                      EffectClass);
				}
			}
			else
			{
				for (UClass* EffectClass : BulletData.DamageEffects)
				{
					UDamageSystemCore::TakeDamageActorToASC(BulletData.Instigator.Get(), this, SweepResult,
					                                        OtherAbilityComp, EffectClass);
				}
			}
		}
		else
		{
			UDamageSystemCore::TakeDamageActorToActor(BulletData.Instigator.Get(), this, OtherActor, BulletData.BaseDamage,
													  BulletData.DamageType);	
		}
	}
}

void ABulletBase::OnProjectileHit_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 const FHitResult& SweepResult)
{
	Super::OnProjectileHit_Implementation(OverlappedComponent, OtherActor, SweepResult);
}
