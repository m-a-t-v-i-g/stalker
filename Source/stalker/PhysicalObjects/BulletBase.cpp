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
	
	BulletData.AmmoObject = Ammo;
	BulletData.Owner = GetOwner();
	BulletData.Instigator = GetInstigator();

	if (BulletData.Instigator.IsValid())
	{
		BulletData.OwnerAbilityComponent = BulletData.Instigator->GetComponentByClass<UAbilitySystemComponent>();
	}

	BulletData.DamageType = Ammo->GetDamageType();
	BulletData.DamageValue = Weapon->GetDamageData().DamageMultiplier * Ammo->GetDamageData().BaseDamage;
}

void ABulletBase::HitLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		FApplyDamageData DamageData;
		DamageData.SetInstigator(BulletData.Instigator.Get(), this);
		DamageData.SetDamageInfo(BulletData.DamageType, BulletData.DamageValue);
		DamageData.SetHitResult(SweepResult);
		DamageData.SetSourceObject(BulletData.AmmoObject.Get());
		
		if (auto OtherAbilityComp = OtherActor->GetComponentByClass<UAbilitySystemComponent>())
		{
			DamageData.SetTargetASC(OtherAbilityComp);
			
			if (BulletData.OwnerAbilityComponent.IsValid())
			{
				DamageData.SetSourceASC(BulletData.OwnerAbilityComponent.Get());
				
				FActiveGameplayEffectHandle GameplayEffectHandle;
				
				UDamageSystemCore::TakeDamageASCtoASC(DamageData, GameplayEffectHandle);
			}
			else
			{
				FActiveGameplayEffectHandle GameplayEffectHandle;
				
				UDamageSystemCore::TakeDamageActorToASC(DamageData, GameplayEffectHandle);
			}
		}
		else
		{
			float OutDamageValue;
			
			UDamageSystemCore::TakeDamageActorToActor(DamageData, OutDamageValue);	
		}
	}
}

void ABulletBase::OnProjectileHit_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 const FHitResult& SweepResult)
{
	Super::OnProjectileHit_Implementation(OverlappedComponent, OtherActor, SweepResult);
}
