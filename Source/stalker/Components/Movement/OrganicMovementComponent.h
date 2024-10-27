// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenOrganicMovementComponent.h"
#include "MovementModelConfig.h"
#include "OrganicMovementComponent.generated.h"

class UMovementModelConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOrganicJumped);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UOrganicMovementComponent : public UGenOrganicMovementComponent
{
	GENERATED_BODY()

public:
	UOrganicMovementComponent();

	UPROPERTY(BlueprintReadOnly, Category = "Character Movement|Movement")
	EOrganicGait AllowedGait = EOrganicGait::Walk;

	UPROPERTY(BlueprintAssignable)
	FOnOrganicJumped OnJumpedDelegate;

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	virtual void BindReplicationData_Implementation() override;

	virtual void PreMovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdateSimulated_Implementation(float DeltaSeconds) override;

	virtual void OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode) override;
	virtual void OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode) override;
	
#pragma region Movement

	void SetMovementModel(UMovementModelConfig* ModelConfig);
	
	void SetMovementSettings(FMovementModel_Settings NewMovementSettings);
	FMovementModel_Settings GetMovementSettings() const;

	void SetMovementState(const EOrganicMovementState NewMovementState, bool bForce = false);
	FORCEINLINE EOrganicMovementState GetMovementState() const { return MovementStatus; }
	FORCEINLINE EOrganicMovementState GetPrevMovementState() const { return PrevMovementState; }

	void OnMovementStateChanged(const EOrganicMovementState PreviousState);

#pragma endregion Movement

#pragma region Rotation

	void SetInputRotationMode(EOrganicRotationMode NewInputRotationMode);
	FORCEINLINE EOrganicRotationMode GetInputRotationMode() const { return InputRotationMode; }

	void SetRotationMode(EOrganicRotationMode NewRotationMode, bool bForce = false);
	FORCEINLINE EOrganicRotationMode GetRotationMode() const { return RotationMode; }

	void OnRotationModeChanged(EOrganicRotationMode PrevRotationMode);

#pragma endregion Rotation

#pragma region Stance

	void SetInputStance(EOrganicStance NewInputStance);
	FORCEINLINE EOrganicStance GetInputStance() const { return InputStance; }
	
	void SetStance(const EOrganicStance NewStance, bool bForce = false);
	FORCEINLINE EOrganicStance GetStance() const { return Stance; }

	virtual void OnStanceChanged(EOrganicStance PreviousStance);
	
#pragma endregion Stance

#pragma region Gait

	void SetInputGait(EOrganicGait NewInputGait);
	FORCEINLINE EOrganicGait GetInputGait() const { return InputGait; }

	void SetGait(const EOrganicGait NewGait, bool bForce = false);
	FORCEINLINE EOrganicGait GetGait() const { return Gait; }
	
	void OnGaitChanged(EOrganicGait PreviousGait);

	void UpdateGait();
	
	EOrganicGait CalculateAllowedGait() const;
	EOrganicGait CalculateActualGait(EOrganicGait NewAllowedGait) const;
	
	void SetAllowedGait(EOrganicGait DesiredGait);

#pragma endregion Gait

	void ForceUpdateAllStates();

	float CalculateGroundRotationRate() const;

	float GetAnimCurveValue(FName CurveName) const;
	
	float GetMappedSpeed() const;

	void UpdateGroundRotation(float DeltaTime);
	void UpdateAirborneRotation(float DeltaTime);
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
	void RotateRootCollision(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
	
	FORCEINLINE FVector GetInstantAcceleration() const { return InstantAcceleration; }

	FORCEINLINE FRotator GetInputRotation() const { return ViewRotation; }
	
	FORCEINLINE FRotator GetLastPawnRotation() const { return PrevPawnRotation; }
	
	FORCEINLINE FRotator GetLastComponentRotation() const { return PrevComponentRotation; }
	
	FORCEINLINE float GetViewYawRate() const { return ViewYawRate; }

	FORCEINLINE float GetMovementInputValue() const { return MovementInputValue; }

	FORCEINLINE bool HasMovementInput() const { return bHasMovementInput; }

	FORCEINLINE bool IsMoving() const { return bIsMoving; }
	
protected:
	TObjectPtr<class ABaseOrganic> OrganicOwner;

	UPROPERTY(EditDefaultsOnly, Category = "Jump")
	float JumpForce = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float SpeedInterpSpeed = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "View")
	float ViewInterpSpeed = 10.0f;
	
#pragma region Movement

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UMovementModelConfig> MovementModel;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FMovementModel_Settings MovementSettings;

	UPROPERTY(VisibleInstanceOnly, Category = "Movement")
	FOrganicMovementState MovementStatus = EOrganicMovementState::None;

	UPROPERTY(VisibleInstanceOnly, Category = "Movement")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;
	
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
	EOrganicGait InputGait = EOrganicGait::Run;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Gait")
	FOrganicGait Gait = EOrganicGait::Run;

#pragma endregion Gait

	float TargetMaxSpeed = 0.0f;
	
	void CalculateMovement(float DeltaSeconds);
	
	void Sprinting();
	void Crouching();
	void Jumping();

	virtual bool CanSprint() const;
	virtual bool CanCrouch() const;
	virtual bool CanJump() const;
	
	void OnJump();
	void OnCrouch();
	void OnUnCrouch();
	void OnSprint(bool bEnabled);
	
private:
	bool bWantsChangeMovementSettings = false;

	FVector InstantAcceleration;

	FVector PreviousVelocity;
	
	FRotator ViewRotation;

	float ViewYawRate = 0.0f;
	
	float PreviousViewYaw = 0.0f;

	FRotator TargetRotation;
	
	FRotator VelocityRotation;

	FRotator PrevPawnRotation;
	
	FRotator PrevComponentRotation;

	float MovementInputValue = 0.0f;

	bool bHasMovementInput = false;
	
	bool bIsMoving = false;

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
};
