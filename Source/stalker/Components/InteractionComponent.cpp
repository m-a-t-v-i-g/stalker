// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionComponent.h"
#include "InteractableInterface.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::Interact()
{
	FHitResult HitResult;
	
	FVector ViewPoint;
	FRotator ViewRotation;

	GetOwner()->GetActorEyesViewPoint(ViewPoint, ViewRotation);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Visibility);
	
	FVector StartPoint = ViewPoint;
	FVector EndPoint = StartPoint + ViewRotation.Vector() * 1000.0f;

	bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, StartPoint, EndPoint, ObjectQueryParams);
	AActor* HitActor = HitResult.GetActor();

	DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 2.0f, 0, 3.5f);

	if (!HitActor)
	{
		return;
	}
	
	if (bHit)
	{
		if (auto InteractableActor = Cast<IInteractableInterface>(HitActor))
		{
			InteractableActor->OnInteract();
		}
	}
}

void UInteractionComponent::ServerInteract_Implementation()
{
	Interact();
}
