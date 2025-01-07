// Fill out your copyright notice in the Description page of Project Settings.

#include "PawnInteractionComponent.h"
#include "InteractableInterface.h"

UPawnInteractionComponent::UPawnInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UPawnInteractionComponent::SetupInteractionComponent()
{
	if (auto Pawn = GetOwner<APawn>())
	{
		PawnRef = Pawn;
		
		if (AController* Controller = Pawn->GetController())
		{
			ControllerRef = Controller;
		}
	}
}

void UPawnInteractionComponent::AddPossibleInteraction(AActor* NewActor)
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
		OnPossibleInteractionAdd.Broadcast(NewActor);
	}
}

void UPawnInteractionComponent::RemovePossibleInteraction(AActor* OldActor)
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
		OnPossibleInteractionRemove.Broadcast(OldActor);
	}
}

void UPawnInteractionComponent::Interact()
{
	if (!bActive)
	{
		return;
	}
	
	if (AActor* ActorUnderTrace = GetActorUnderLineTrace())
	{
		if (ActorUnderTrace->Implements<UInteractableInterface>())
		{
			if (!PawnRef->HasAuthority() && PawnRef->IsLocallyControlled())
			{
				PreInteractionDelegate.Broadcast(ActorUnderTrace);
			}

			ServerInteract(ActorUnderTrace);
		}
	}
}

void UPawnInteractionComponent::ServerInteract_Implementation(AActor* ActorToInteract)
{
	if (auto InteractableActor = Cast<IInteractableInterface>(ActorToInteract))
	{
		PreInteractionDelegate.Broadcast(ActorToInteract);
		InteractableActor->OnInteract(GetOwner());
	}
}

void UPawnInteractionComponent::UpdateFindActorTimer()
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

void UPawnInteractionComponent::FindInteract()
{
	if (!bActive)
	{
		return;
	}

	AActor* FoundActor = GetActorUnderLineTrace();
	
	if (DetectedActor != FoundActor)
	{
		DetectedActor = FoundActor;
		OnDetectedActorChange.Broadcast(DetectedActor.Get());
	}
}

AActor* UPawnInteractionComponent::GetActorUnderLineTrace() const
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
