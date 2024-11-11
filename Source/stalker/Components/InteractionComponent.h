// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InteractionComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDetectedActorChanged, const AActor*);

UCLASS()
class STALKER_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnDetectedActorChanged OnDetectedActorChanged;
	
	UInteractionComponent();

	void SetupInteractionComponent();

	void AddPossibleInteraction(AActor* NewActor);
	void RemovePossibleInteraction(AActor* OldActor);

	AActor* GetDetectedActor() const { return DetectedActor.Get(); }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float DetectionRate = 0.125f;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact();

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* ActorToInteract);

	void UpdateFindActorTimer();
	
	void FindInteract();

	AActor* GetActorUnderLineTrace() const;
	
private:
	TObjectPtr<APawn> PawnRef;
	TObjectPtr<AController> ControllerRef;

	UPROPERTY(VisibleInstanceOnly, Category = "Interaction")
	TArray<AActor*> PossibleInteractions;

	TWeakObjectPtr<AActor> DetectedActor;
	
	bool bActive = false;

	FTimerHandle FindActorTimer;
};
