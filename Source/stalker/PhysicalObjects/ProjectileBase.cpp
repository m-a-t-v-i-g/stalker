// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBase.h"
#include "Ammo/AmmoObject.h"
#include "Components/SphereComponent.h"
#include "DamageSystem/DamageSystemCore.h"
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

	SetReplicates(true);
	SetReplicatingMovement(true);
	
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PhysicsRoot->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnBulletBeginOverlap);
}

void AProjectileBase::SetupProjectile(const UWeaponObject* Weapon, const UAmmoObject* Ammo)
{
	WeaponObjectRef = Weapon;
	AmmoObjectRef = Ammo;
}

void AProjectileBase::OnBulletBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                           const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || ActorsToIgnore.Contains(OtherActor))
	{
		return;
	}

	if (HasAuthority())
	{
		float Damage = UDamageSystemCore::CalculateProjectileDamage(AmmoObjectRef->GetDamageData().BaseDamage,
		                                                            WeaponObjectRef->GetDamageData().DamageMultiplier);
		FVector ShotDirection = GetActorLocation() - GetInstigator()->GetActorLocation();
		
		OtherActor->TakeDamage(
			Damage, FPointDamageEvent(Damage, SweepResult, ShotDirection.GetSafeNormal(), AmmoObjectRef->GetDamageType()),
			GetInstigatorController(), this);
		
		Destroy();
	}
}
