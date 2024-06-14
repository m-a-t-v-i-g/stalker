// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenOrganicMovementComponent.h"
#include "OrganicMovementComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STALKER_API UOrganicMovementComponent : public UGenOrganicMovementComponent
{
	GENERATED_BODY()

public:
	UOrganicMovementComponent();

	virtual void PostLoad() override;
	
	virtual void BindReplicationData_Implementation() override;

	virtual void MovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdateSimulated_Implementation(float DeltaSeconds) override;

	virtual void OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode) override;
	virtual void OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode) override;

protected:
	TObjectPtr<class ABaseOrganic> OrganicOwner;

private:
	UPROPERTY(EditDefaultsOnly, Category = "View")
	float ViewInterpSpeed = 30.0f;
	
	FRotator ViewRotation;
	FRotator PrevPawnRotation;
	FRotator PrevComponentRotation;

public:
	FRotator GetViewRotation() const { return ViewRotation; }
};
