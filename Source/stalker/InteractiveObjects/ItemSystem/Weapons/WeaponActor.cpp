// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "WeaponObject.h"
#include "Ammo/AmmoObject.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicalObjects/BulletBase.h"

AWeaponActor::AWeaponActor()
{
	Muzzle = CreateDefaultSubobject<USceneComponent>("Muzzle");
	Muzzle->SetupAttachment(GetMesh());
}

void AWeaponActor::OnBindItem()
{
	Super::OnBindItem();

	if (auto WeaponObject = GetItemObject<UWeaponObject>())
	{
		WeaponObject->OnAttackStart.AddDynamic(this, &AWeaponActor::OnStartAttack);
		WeaponObject->OnAttackStop.AddDynamic(this, &AWeaponActor::OnStopAttack);
	}
}

void AWeaponActor::OnUnbindItem(UItemObject* PrevItemObject)
{
	Super::OnUnbindItem(PrevItemObject);

	if (auto WeaponObject = Cast<UWeaponObject>(PrevItemObject))
	{
		WeaponObject->OnAttackStart.RemoveAll(this);
		WeaponObject->OnAttackStop.RemoveAll(this);
	}
}

void AWeaponActor::OnStartAttack()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		MakeAttackVisual();
	}
	
	if (HasAuthority())
	{
		SpawnBullet();
		MulticastMakeAttackVisual();
	}
}

void AWeaponActor::MakeAttackVisual()
{
	UKismetSystemLibrary::PrintString(this, FString("ATTACK"), true, false, FLinearColor::Green);
}

void AWeaponActor::MulticastMakeAttackVisual_Implementation()
{
	if (GetLocalRole() != ROLE_AutonomousProxy)
	{
		MakeAttackVisual();
	}
}

void AWeaponActor::OnStopAttack()
{
	
}

void AWeaponActor::OnStartAlternative()
{
}

void AWeaponActor::OnStopAlternative()
{
}

ABulletBase* AWeaponActor::SpawnBullet()
{
	if (UWeaponObject* WeaponObject = GetWeaponObject())
	{
		TArray<UAmmoObject*> Cartridge = WeaponObject->GetRounds();

		if (!Cartridge.IsEmpty())
		{
			if (UAmmoObject* CurrentAmmo = Cartridge[0])
			{
				check(Muzzle);

				FRotator BulletRotation = FRotationMatrix::MakeFromX(GetFireLocation() - Muzzle->GetComponentLocation()).Rotator();
				FTransform SpawnTransform = Muzzle->GetComponentTransform();
				SpawnTransform.SetRotation(FQuat(BulletRotation));
				
				if (ABulletBase* Bullet = GetWorld()->SpawnActorDeferred<ABulletBase>(
					CurrentAmmo->GetBulletClass(), SpawnTransform, this, GetInstigator(),
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
				{
					Bullet->SetupBullet(GetWeaponObject(), CurrentAmmo);
					OnSetupBullet(Bullet);
					
					Bullet->FinishSpawning(SpawnTransform);
					return Bullet;
				}
			}
		}
	}
	return nullptr;
}

void AWeaponActor::OnSetupBullet(ABulletBase* Bullet)
{
	TArray<AActor*> ActorsToIgnore {this, GetOwner()};
	Bullet->ActorsToIgnore = ActorsToIgnore;
}

FVector AWeaponActor::GetFireLocation()
{
	if (AController* Controller = GetInstigatorController())
	{
		FHitResult HitResult;
		FVector ViewPoint;
		FRotator ViewRotation;

		Controller->GetPlayerViewPoint(ViewPoint, ViewRotation);
		
		FVector StartPoint = ViewPoint;
		FVector EndPoint = StartPoint + ViewRotation.Vector() * 10000.0f;

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Visibility);
		return bHit ? HitResult.Location : HitResult.TraceEnd;
	}
	return FVector();
}
