// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerActor.h"
#include "InteractorInterface.h"
#include "Components/InventoryComponent.h"

FName AContainerActor::InventoryComponentName {"Inventory Component"};

AContainerActor::AContainerActor()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	
	PrimaryActorTick.bCanEverTick = false;
}

bool AContainerActor::OnInteract(AActor* Interactor)
{
	if (Interactor)
	{
		if (auto InteractorInterface = Cast<IInteractorInterface>(Interactor))
		{
			InteractorInterface->InteractWithContainer(InventoryComponent);
			return true;
		}
	}
	return false;
}
