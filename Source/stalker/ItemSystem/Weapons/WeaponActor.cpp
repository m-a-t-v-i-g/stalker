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
		WeaponObject->OnAttackStart.AddUObject(this, &AWeaponActor::OnFireStart);
		WeaponObject->OnAttackStop.AddUObject(this, &AWeaponActor::OnFireStop);
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

void AWeaponActor::OnFireStart()
{
}

void AWeaponActor::OnFireStop()
{
	
}

UWeaponObject* AWeaponActor::GetWeaponObject() const
{
	return Cast<UWeaponObject>(GetItemObject());
}
