// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "Components/Items/ItemsContainerComponent.h"

AStalkerCharacter::AStalkerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStalkerCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	InventoryComponent = GetComponentByClass<UInventoryComponent>();
}
