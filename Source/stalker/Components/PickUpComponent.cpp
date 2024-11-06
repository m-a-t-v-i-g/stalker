// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUpComponent.h"

UPickUpComponent::UPickUpComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPickUpComponent::SetupPickUpComponent(UInteractionComponent* InteractionComp, UInventoryComponent* InventoryComp)
{
	if (!InteractionComp || !InventoryComp)
	{
		return;
	}

	InteractionComponent = InteractionComp;
	TargetInventory = InventoryComp;
}

bool UPickUpComponent::TryPickUpItem(AItemActor* ItemActor)
{
	return true;
}
