// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/Inventory/InventoryComponent.h"

AStalkerCharacter::AStalkerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStalkerCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	AbilityComponent = GetComponentByClass<UStalkerAbilityComponent>();
	InventoryComponent = GetComponentByClass<UInventoryComponent>();
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (GetAbilityComponent())
	{
		GetAbilityComponent()->InitAbilitySystem(NewController, this);
	}
}
