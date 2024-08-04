// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWeaponActor::InitItem(UItemObject* NewItemObject)
{
	Super::InitItem(NewItemObject);
}
