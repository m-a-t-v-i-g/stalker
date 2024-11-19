// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveActor.h"
#include "Components/PawnInteractionComponent.h"
#include "Components/SphereComponent.h"

AInteractiveActor::AInteractiveActor()
{
	InteractionSphere = CreateDefaultSubobject<USphereComponent>("Interaction Sphere");
	InteractionSphere->SetupAttachment(GetRootComponent());
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	
	PrimaryActorTick.bCanEverTick = false;
}

bool AInteractiveActor::OnInteract(AActor* Interactor)
{
	return true;
}

void AInteractiveActor::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                        bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto InteractionComp = OtherActor->GetComponentByClass<UPawnInteractionComponent>())
		{
			InteractionComp->AddPossibleInteraction(this);
		}
	}
}

void AInteractiveActor::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (auto InteractionComp = OtherActor->GetComponentByClass<UPawnInteractionComponent>())
		{
			InteractionComp->RemovePossibleInteraction(this);
		}
	}
}

void AInteractiveActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractiveActor::OnInteractionSphereEndOverlap);
}
