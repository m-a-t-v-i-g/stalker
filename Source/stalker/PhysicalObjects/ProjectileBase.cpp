// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"

FName AProjectileBase::ProjectileMovementComponentName {"Projectile Movement"};

AProjectileBase::AProjectileBase()
{
	PhysicsRoot = CreateDefaultSubobject<USphereComponent>("Physics Root");
	SetRootComponent(PhysicsRoot);
	PhysicsRoot->CanCharacterStepUpOn = ECB_No;
	PhysicsRoot->SetCollisionProfileName("Bullet");
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionProfileName("IgnoreAllDynamic");
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(ProjectileMovementComponentName);
	if (PhysicsRoot)
	{
		ProjectileMovement->UpdatedComponent = GetRootComponent();
	}

	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PhysicsRoot->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnBulletBeginOverlap);
}

void AProjectileBase::OnBulletBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                           const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || ActorsToIgnore.Contains(OtherActor))
	{
		return;
	}

	OtherActor->TakeDamage(0.0f, FDamageEvent(), GetInstigatorController(), this);
	
	Destroy();
}
