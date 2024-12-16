// Fill out your copyright notice in the Description page of Project Settings.

#include "ExplosionArea.h"
#include "Components/SphereComponent.h"

AExplosionArea::AExplosionArea()
{
	ExplosionZone = CreateDefaultSubobject<USphereComponent>("Explosion Zone");
	ExplosionZone->SetupAttachment(GetRootComponent());
	ExplosionZone->CanCharacterStepUpOn = ECB_No;
	ExplosionZone->SetCollisionProfileName("OverlapAllDynamic");

	PrimaryActorTick.bCanEverTick = false;
}
