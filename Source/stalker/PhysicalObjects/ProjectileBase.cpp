// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

FName AProjectileBase::ProjectileMovementComponentName {"Projectile Movement"};

AProjectileBase::AProjectileBase()
{
	PhysicsRoot = CreateDefaultSubobject<USphereComponent>("Physics Root");
	SetRootComponent(PhysicsRoot);
	PhysicsRoot->CanCharacterStepUpOn = ECB_No;
	PhysicsRoot->SetCollisionProfileName("Projectile");
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionProfileName("IgnoreAllDynamic");
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(ProjectileMovementComponentName);
	if (PhysicsRoot)
	{
		ProjectileMovement->UpdatedComponent = GetRootComponent();
	}

	SetCanBeDamaged(false);
	AActor::SetLifeSpan(6.0f);
}

void AProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PhysicsRoot->OnComponentHit.AddDynamic(this, &AProjectileBase::OnProjectileHitTarget);
	PhysicsRoot->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnProjectileOverlapTarget);
}

void AProjectileBase::OnProjectileHitTarget(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                            const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || ActorsToIgnore.Contains(OtherActor))
	{
		return;
	}

	ActorsToIgnore.AddUnique(OtherActor);

	HitLogic(OtherComp, OtherActor, Hit);
	Destroy();
}

void AProjectileBase::OnProjectileOverlapTarget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || ActorsToIgnore.Contains(OtherActor))
	{
		return;
	}

	ActorsToIgnore.AddUnique(OtherActor);

	OverlapLogic(OtherComp, OtherActor, SweepResult);
	Destroy();
}

void AProjectileBase::HitLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              const FHitResult& SweepResult)
{
}

void AProjectileBase::OverlapLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  const FHitResult& SweepResult)
{
}
