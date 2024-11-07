// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "ContainerActor.generated.h"

UCLASS()
class STALKER_API AContainerActor : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AContainerActor();

	virtual bool OnInteract(AActor* Interactor) override;
};
