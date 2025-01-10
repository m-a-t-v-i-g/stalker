// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemSpawner.h"

#include "ItemActor.h"

AItemSpawner::AItemSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AItemSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		auto Item = GetWorld()->SpawnActor<AItemActor>(SpawnActorClass, SpawnParameters);
		if (Item)
		{
			
		}
	}
}
