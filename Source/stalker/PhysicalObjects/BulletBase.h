// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "BulletBase.generated.h"

class UGameplayEffect;
class UWeaponObject;
class UAbilitySystemComponent;

USTRUCT()
struct FBulletBaseData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<UAmmoObject> AmmoObject;
	TWeakObjectPtr<AActor> Owner;
	TWeakObjectPtr<AActor> Instigator;
	TWeakObjectPtr<UAbilitySystemComponent> OwnerAbilityComponent;
	
	TSubclassOf<UDamageType> DamageType;
	float BaseDamage = 0.0f;
	TSubclassOf<UGameplayEffect> DamageEffect;
};

UCLASS()
class STALKER_API ABulletBase : public AProjectileBase
{
	GENERATED_BODY()

public:
	ABulletBase();
	
	virtual void SetupBullet(UWeaponObject* Weapon, UAmmoObject* Ammo);

	virtual void HitLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                     const FHitResult& SweepResult) override;
	virtual void OnProjectileHit_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                            const FHitResult& SweepResult) override;

	FBulletBaseData GetBulletData() const { return BulletData; }
	
protected:
	
private:
	FBulletBaseData BulletData;
};
