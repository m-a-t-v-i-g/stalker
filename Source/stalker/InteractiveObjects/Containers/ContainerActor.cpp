// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerActor.h"

#include "InteractorInterface.h"

AContainerActor::AContainerActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AContainerActor::OnInteract(AActor* Interactor)
{
	if (Interactor)
	{
		if (auto InteractorInterface = Cast<IInteractorInterface>(Interactor))
		{
			if (InteractorInterface->ContainerInteract())
			{
				return true;
			}
		}
	}
	return false;
}
