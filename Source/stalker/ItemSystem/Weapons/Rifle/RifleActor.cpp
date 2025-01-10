// Fill out your copyright notice in the Description page of Project Settings.

#include "RifleActor.h"
#include "PhysicalObjects/ProjectileBase.h"

ARifleActor::ARifleActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARifleActor::OnBindItem()
{
	Super::OnBindItem();
}

void ARifleActor::OnUnbindItem(UItemObject* PrevItemObject)
{
	Super::OnUnbindItem(PrevItemObject);
}

void ARifleActor::OnFireStart()
{
	Super::OnFireStart();
}

void ARifleActor::OnFireStop()
{
	Super::OnFireStop();
}
