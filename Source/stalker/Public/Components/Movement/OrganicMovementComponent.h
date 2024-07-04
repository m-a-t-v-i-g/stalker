// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenOrganicMovementComponent.h"
#include "Library/OrganicLibrary.h"
#include "OrganicMovementComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STALKER_API UOrganicMovementComponent : public UGenOrganicMovementComponent
{
	GENERATED_BODY()

public:
	UOrganicMovementComponent();

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	virtual void BindReplicationData_Implementation() override;

	virtual void MovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdateSimulated_Implementation(float DeltaSeconds) override;

	virtual void OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode) override;
	virtual void OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode) override;

protected:
	TObjectPtr<class ABaseOrganic> OrganicOwner;

public:
	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement|Movement")
	EOrganicGait AllowedGait = EOrganicGait::Slow;

	UPROPERTY(BlueprintReadOnly, Category = "CharacterMovement|Movement")
	FOrganicMovementSettings MovementSettings;

private:
	UPROPERTY(EditDefaultsOnly, Category = "View")
	float ViewInterpSpeed = 30.0f;
	
	FRotator ViewRotation;
	FRotator PrevPawnRotation;
	FRotator PrevComponentRotation;

public:
	void SetMovementSettings(FOrganicMovementSettings NewMovementSettings);
	void SetAllowedGait(EOrganicGait DesiredGait);

	float GetMappedSpeed() const;

	FORCEINLINE FRotator GetViewRotation() const { return ViewRotation; }
	
	FORCEINLINE FRotator GetLastPawnRotation() const { return PrevPawnRotation; }
	
	FORCEINLINE FRotator GetLastComponentRotation() const { return PrevComponentRotation; }
};
