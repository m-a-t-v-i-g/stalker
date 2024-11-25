// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "InteractiveActor.generated.h"

class USphereComponent;

UCLASS()
class STALKER_API AInteractiveActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AInteractiveActor();

	virtual bool OnInteract(AActor* Interactor) override;
	
	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
										 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
										 const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
									   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	USphereComponent* GetInteractionSphere() const { return InteractionSphere; }
	
protected:
	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> InteractionSphere;
};
