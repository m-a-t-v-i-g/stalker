// Fill out your copyright notice in the Description page of Project Settings.

#include "BulletBase.h"
#include "AbilitySystemComponent.h"
#include "Ammo/AmmoObject.h"
#include "Components/SphereComponent.h"
#include "DamageSystem/DamageSystemCore.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Weapons/WeaponObject.h"

ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABulletBase::SetupBullet(UWeaponObject* Weapon, UAmmoObject* Ammo)
{
	check(Weapon);
	check(Ammo);
	
	BulletData.SourceAmmo = Ammo;
	BulletData.Owner = GetOwner();
	BulletData.Instigator = GetInstigator();
	BulletData.DamageType = Ammo->GetDamageType();
	BulletData.DamageValue = Weapon->GetDamageData().DamageMultiplier * Ammo->GetDamageData().BaseDamage;
	BulletData.SweepRadius = Ammo->GetBulletSweepRadius();
	BulletData.Speed = Ammo->GetBulletSpeed() * Weapon->GetBulletSpeedMultiplier() * 100.0f;
	BulletData.Nastiness = (1 - Ammo->GetBulletNastiness()) / Weapon->GetNastinessMultiplier();

	if (GetPhysicsRoot())
	{
		GetPhysicsRoot()->SetSphereRadius(BulletData.SweepRadius);
	}

	if (GetProjectileMovement())
	{
		GetProjectileMovement()->InitialSpeed = BulletData.Speed;
		GetProjectileMovement()->MaxSpeed = BulletData.Speed * 0.98;
		GetProjectileMovement()->ProjectileGravityScale = BulletData.Nastiness;
	}
}

void ABulletBase::HitLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		FApplyDamageData DamageData = UDamageSystemCore::GenerateDamageData(
			BulletData.Instigator.Get(), OtherActor, this, BulletData.DamageType, BulletData.DamageValue, SweepResult,
			BulletData.SourceAmmo.Get());

		UDamageSystemCore::TakeDamage(DamageData);
	}
}

void ABulletBase::OverlapLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		FApplyDamageData DamageData = UDamageSystemCore::GenerateDamageData(
			BulletData.Instigator.Get(), OtherActor, this, BulletData.DamageType, BulletData.DamageValue, SweepResult,
			BulletData.SourceAmmo.Get());

		UDamageSystemCore::TakeDamage(DamageData);
	}
}
