// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenOrganicMovementComponent.h"
#include "Library/OrganicLibrary.h"
#include "OrganicMovementComponent.generated.h"

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UOrganicMovementComponent : public UGenOrganicMovementComponent
{
	GENERATED_BODY()

public:
	UOrganicMovementComponent();

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	virtual void BindReplicationData_Implementation() override;

	virtual void PreMovementUpdate_Implementation(float DeltaSeconds) override;
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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Jump")
	float JumpForce = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float SpeedInterpSpeed = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "View")
	float ViewInterpSpeed = 10.0f;
	
private:
	bool bWantsChangeMovementSettings = false;
	
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

protected:
	float TargetMaxSpeed = 0.0f;
	
private:
	bool bWantsToSprint = false;
	bool bWantsToCrouch = false;
	bool bWantsToJump = false;

	bool bJustSprinting = false;
	bool bJustCrouched = false;
	bool bJustJumped = false;

	bool bCanSprint = false;
	bool bCanCrouch = false;
	bool bCanJump = false;

	void UpdateSprint(bool bRequestedSprint);
	void UpdateCrouch(bool bRequestedCrouch);
	void UpdateJump(bool bRequestedJump);

protected:
	void Sprinting();
	void Crouching();
	void Jumping();

	virtual bool CanSprint() const;
	virtual bool CanCrouch() const;
	virtual bool CanJump() const;
	
public:
	void OnJump();
	void OnCrouch();
	void OnUnCrouch();
};
