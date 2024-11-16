// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "WeaponObject.h"
#include "Ammo/AmmoObject.h"
#include "Kismet/KismetSystemLibrary.h"

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
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
