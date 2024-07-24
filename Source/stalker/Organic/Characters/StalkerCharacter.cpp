// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "Components/Inventory/CharacterInventoryComponent.h"

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.Get())
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStalkerCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	InventoryComponent = GetComponentByClass<UCharacterInventoryComponent>();
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (GetInventoryComponent())
	{
		GetInventoryComponent()->InitializeContainer();
	}
}
