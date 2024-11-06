// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InteractionComponent.generated.h"

UCLASS()
class STALKER_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	void SetupInteractionComponent();
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact();

private:
	TObjectPtr<APawn> PawnRef;
	TObjectPtr<AController> ControllerRef;
	
	bool bActive = false;
};
