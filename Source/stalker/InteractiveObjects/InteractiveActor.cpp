// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveActor.h"
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
