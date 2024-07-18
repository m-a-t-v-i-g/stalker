// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/Mobs/BaseDog.h"
#include "FlatCapsuleComponent.h"

ABaseDog::ABaseDog(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer
	.SetDefaultSubobjectClass<UFlatCapsuleComponent>(CapsuleName))
{
	PrimaryActorTick.bCanEverTick = true;
}
