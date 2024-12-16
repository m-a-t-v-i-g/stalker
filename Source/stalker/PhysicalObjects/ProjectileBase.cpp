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
	AActor::SetLifeSpan(3.0f);
}

void AProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PhysicsRoot->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnProjectileOverlapTarget);
}

void AProjectileBase::OnProjectileOverlapTarget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || ActorsToIgnore.Contains(OtherActor))
	{
		return;
	}

	HitLogic(OverlappedComponent, OtherActor, SweepResult);
	OnProjectileHit(OverlappedComponent, OtherActor, SweepResult);

	ActorsToIgnore.AddUnique(OtherActor);

	Destroy();
}

void AProjectileBase::HitLogic_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              const FHitResult& SweepResult)
{
}

void AProjectileBase::OnProjectileHit_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     const FHitResult& SweepResult)
{
}
