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

	virtual void PhysicsGrounded(float DeltaSeconds) override;
	virtual void PhysicsAirborne(float DeltaSeconds) override;
	
	virtual void PreMovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdateSimulated_Implementation(float DeltaSeconds) override;

	virtual void OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode) override;
	virtual void OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode) override;

protected:
	TObjectPtr<class ABaseOrganic> OrganicOwner;

	//TObjectPtr<AActor> OrganicOwner;

#pragma region Movement

	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic|Movement")
	FDataTableRowHandle MovementTable;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic|Movement")
	FOrganicMovementModel MovementModel;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Movement")
	FOrganicMovementState MovementState = EOrganicMovementState::None;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Movement")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;
	*/
	
#pragma endregion Movement

#pragma region Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Organic|Rotation")
	EOrganicRotationMode InputRotationMode = EOrganicRotationMode::VelocityDirection;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Rotation")
	FOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;
	
#pragma endregion Rotation

#pragma region Stance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Organic|Stance")
	EOrganicStance InputStance = EOrganicStance::Standing;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Stance")
	FOrganicStance Stance = EOrganicStance::Standing;

#pragma endregion Stance

#pragma region Gait

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Organic|Gait")
	EOrganicGait InputGait = EOrganicGait::Medium;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Gait")
	FOrganicGait Gait = EOrganicGait::Medium;

#pragma endregion Gait

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

	FVector InstantAcceleration;

	FVector PreviousVelocity;
	
	FRotator ViewRotation;

	float ViewYawRate;
	
	float PreviousViewYaw;

	FRotator TargetRotation;
	
	FRotator VelocityRotation;

	FRotator PrevPawnRotation;
	
	FRotator PrevComponentRotation;

	float MovementInputValue = 0.0f;

	bool bHasMovementInput = false;
	
	bool bIsMoving = false;

public:
	void SetMovementSettings(FOrganicMovementSettings NewMovementSettings);

	void UpdateGait();
	
	EOrganicGait CalculateAllowedGait() const;
	EOrganicGait CalculateActualGait(EOrganicGait NewAllowedGait) const;
	
	void SetAllowedGait(EOrganicGait DesiredGait);

	void SetGait(const EOrganicGait NewGait, bool bForce = false);
	FORCEINLINE EOrganicGait GetGait() const { return Gait; }
	
	float CalculateGroundRotationRate() const;

	float GetAnimCurveValue(FName CurveName) const;
	
	float GetMappedSpeed() const;

	void UpdateGroundRotation(float DeltaTime);
	void UpdateAirborneRotation(float DeltaTime);
	void RotateRootCollision(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
	
	FORCEINLINE FVector GetInstantAcceleration() const { return InstantAcceleration; }

	FORCEINLINE FRotator GetInputRotation() const { return ViewRotation; }
	
	FORCEINLINE FRotator GetLastPawnRotation() const { return PrevPawnRotation; }
	
	FORCEINLINE FRotator GetLastComponentRotation() const { return PrevComponentRotation; }
	
	FORCEINLINE EOrganicRotationMode GetRotationMode() const { return RotationMode; }

	FORCEINLINE float GetViewYawRate() const { return ViewYawRate; }

	FORCEINLINE float GetMovementInputValue() const { return MovementInputValue; }

	FORCEINLINE bool HasMovementInput() const { return bHasMovementInput; }

	FORCEINLINE bool IsMoving() const { return bIsMoving; }
	
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
