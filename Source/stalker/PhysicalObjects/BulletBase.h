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

	TWeakObjectPtr<UAmmoObject> SourceAmmo;
	TWeakObjectPtr<AActor> Owner;
	TWeakObjectPtr<AActor> Instigator;
	UClass* DamageType = nullptr;
	float DamageValue = 0.0f;
	float SweepRadius = 0.0f;
	float Speed = 0.0f;
	float Nastiness = 1.0f;
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
	virtual void OverlapLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                     const FHitResult& SweepResult) override;

	FBulletBaseData GetBulletData() const { return BulletData; }
	
private:
	FBulletBaseData BulletData;
};
