// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionComponent.h"
#include "InteractableInterface.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::SetupInteractionComponent()
{
	if (auto Pawn = GetOwner<APawn>())
	{
		PawnRef = Pawn;
		
		if (auto Controller = Pawn->GetController())
		{
			ControllerRef = Controller;
		}
	}
}

void UInteractionComponent::AddPossibleInteraction(AActor* NewActor)
{
	if (!PawnRef || !ControllerRef)
	{
		return;
	}

	if (!PossibleInteractions.Contains(NewActor))
	{
		PossibleInteractions.Add(NewActor);
		
		UpdateFindActorTimer();
		bActive = !PossibleInteractions.IsEmpty();
	}
}

void UInteractionComponent::RemovePossibleInteraction(AActor* OldActor)
{
	if (!PawnRef || !ControllerRef)
	{
		return;
	}

	if (PossibleInteractions.Contains(OldActor))
	{
		PossibleInteractions.Remove(OldActor);
		
		UpdateFindActorTimer();
		bActive = !PossibleInteractions.IsEmpty();
	}
}

void UInteractionComponent::Interact()
{
	if (!bActive)
	{
		return;
	}
	
	if (AActor* ActorUnderTrace = GetActorUnderLineTrace())
	{
		if (ActorUnderTrace->Implements<UInteractableInterface>())
		{
			ServerInteract(ActorUnderTrace);
		}
	}
}

void UInteractionComponent::ServerInteract_Implementation(AActor* ActorToInteract)
{
	if (auto InteractableActor = Cast<IInteractableInterface>(ActorToInteract))
	{
		InteractableActor->OnInteract(GetOwner());
	}
}

void UInteractionComponent::UpdateFindActorTimer()
{
	FindInteract();
	
	if (!PossibleInteractions.IsEmpty() && !FindActorTimer.IsValid())
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this]
		{
			FindInteract();
		});
		
		GetWorld()->GetTimerManager().SetTimer(FindActorTimer, TimerDelegate, DetectionRate, true);
	}
	
	if (PossibleInteractions.IsEmpty()) 
	{
		GetWorld()->GetTimerManager().ClearTimer(FindActorTimer);
	}
}

void UInteractionComponent::FindInteract()
{
	if (!bActive)
	{
		return;
	}

	AActor* FoundActor = GetActorUnderLineTrace();
	
	if (DetectedActor != FoundActor)
	{
		DetectedActor = FoundActor;
		OnDetectedActorChanged.Broadcast(DetectedActor.Get());
	}
}

AActor* UInteractionComponent::GetActorUnderLineTrace() const
{
	if (PawnRef && ControllerRef)
	{
		FHitResult HitResult;

		FVector ViewPoint;
		FRotator ViewRotation;

		ControllerRef->GetPlayerViewPoint(ViewPoint, ViewRotation);

		FVector StartPoint = ViewPoint;
		FVector EndPoint = StartPoint + ViewRotation.Vector() * 10000.0f;

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Visibility))
		{
			AActor* HitActor = HitResult.GetActor();

			if (HitActor && PossibleInteractions.Contains(HitActor))
			{
				return HitActor;
			}
		}
	}
	return nullptr;
}
