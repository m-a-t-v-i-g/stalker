// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/WeaponObject.h"
#include "ProjectileBase.generated.h"

class UAmmoObject;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class STALKER_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	static FName ProjectileMovementComponentName;

	UPROPERTY()
	TArray<AActor*> ActorsToIgnore;

	virtual void PostInitializeComponents() override;

	virtual void SetupProjectile(const UWeaponObject* Weapon, const UAmmoObject* Ammo);

protected:
	TWeakObjectPtr<const UWeaponObject> WeaponObjectRef;
	TWeakObjectPtr<const UAmmoObject> AmmoObjectRef;

	UFUNCTION()
	void OnBulletBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PhysicsRoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
};
