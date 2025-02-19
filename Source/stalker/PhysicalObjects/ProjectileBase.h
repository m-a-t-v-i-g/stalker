﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

protected:
	UFUNCTION()
	void OnProjectileHitTarget(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnProjectileOverlapTarget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void HitLogic(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void OverlapLogic(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, const FHitResult& SweepResult);

	FORCEINLINE USphereComponent* GetPhysicsRoot() const { return PhysicsRoot; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PhysicsRoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
};
