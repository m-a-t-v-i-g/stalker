// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenOrganicMovementComponent.h"
#include "MovementModelConfig.h"
#include "StalkerCharacterMovementComponent.generated.h"

class UMovementModelConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumped);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UStalkerCharacterMovementComponent : public UGenOrganicMovementComponent
{
	GENERATED_BODY()

public:
	UStalkerCharacterMovementComponent();

	UPROPERTY(BlueprintAssignable)
	FOnJumped OnJumpedDelegate;

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	
	virtual void BindReplicationData_Implementation() override;

	virtual void PreMovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdate_Implementation(float DeltaSeconds) override;
	virtual void MovementUpdateSimulated_Implementation(float DeltaSeconds) override;

	virtual void OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode) override;
	virtual void OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode) override;
	
#pragma region Movement State

	void SetMovementModel(const UMovementModelConfig* ModelConfig);
	
	void SetMovementSettings();

	void SetMovementState(const ECharacterMovementState NewMovementState, bool bForce = false);
	FORCEINLINE ECharacterMovementState GetMovementState() const { return CharMovementState; }
	FORCEINLINE ECharacterMovementState GetPrevMovementState() const { return PrevMovementState; }

	void OnMovementStateChanged(const ECharacterMovementState PreviousState);

#pragma endregion Movement State

#pragma region Rotation Mode

	void SetInputRotationMode(ECharacterRotationMode NewInputRotationMode);
	FORCEINLINE ECharacterRotationMode GetInputRotationMode() const { return InputRotationMode; }

	void SetRotationMode(ECharacterRotationMode NewRotationMode, bool bForce = false);
	FORCEINLINE ECharacterRotationMode GetRotationMode() const { return RotationMode; }

	void OnRotationModeChanged(ECharacterRotationMode PrevRotationMode);

#pragma endregion Rotation Mode

#pragma region Stance

	void SetInputStance(ECharacterStanceType NewInputStance);
	FORCEINLINE ECharacterStanceType GetInputStance() const { return InputStance; }
	
	void SetStance(const ECharacterStanceType NewStance, bool bForce = false);
	FORCEINLINE ECharacterStanceType GetStance() const { return Stance; }

	virtual void OnStanceChanged(ECharacterStanceType PreviousStance);
	
#pragma endregion Stance

#pragma region Gait

	void SetInputGait(ECharacterGaitType NewInputGait);
	FORCEINLINE ECharacterGaitType GetInputGait() const { return InputGait; }

	void SetGait(const ECharacterGaitType NewGait, bool bForce = false);
	FORCEINLINE ECharacterGaitType GetGait() const { return Gait; }
	
	void OnGaitChanged(ECharacterGaitType PreviousGait);

	void UpdateGait();
	
	ECharacterGaitType CalculateAllowedGait() const;
	ECharacterGaitType CalculateActualGait(ECharacterGaitType NewAllowedGait) const;
	
#pragma endregion Gait

	void ForceUpdateAllStates();

	float CalculateGroundRotationRate() const;

	float GetAnimCurveValue(FName CurveName) const;
	
	float GetMappedSpeed() const;

	void UpdateGroundRotation(float DeltaTime);
	void UpdateAirborneRotation(float DeltaTime);
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
	void RotateRootCollision(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
	
	virtual bool CanSprint() const;
	virtual bool CanCrouch() const;
	virtual bool CanJump() const;
	
	FORCEINLINE FVector GetInstantAcceleration() const { return InstantAcceleration; }

	FORCEINLINE FRotator GetInputRotation() const { return ViewRotation; }
	
	FORCEINLINE FRotator GetLastPawnRotation() const { return PrevPawnRotation; }
	
	FORCEINLINE FRotator GetLastComponentRotation() const { return PrevComponentRotation; }
	
	FORCEINLINE float GetViewYawRate() const { return ViewYawRate; }

	FORCEINLINE float GetMovementInputValue() const { return MovementInputValue; }

	FORCEINLINE bool HasMovementInput() const { return bHasMovementInput; }

	FORCEINLINE bool IsMoving() const { return bIsMoving; }
	
	FORCEINLINE bool IsSprinting() const { return bJustSprinting; }
	
	virtual bool IsCrouching() const override { return bJustCrouched; }
	
	FORCEINLINE bool IsJumping() const { return bJustJumped; }
	
protected:
	TObjectPtr<class ABaseCharacter> CharacterOwner;

	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Defaults")
	float JumpForce = 500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Defaults")
	float SpeedInterpSpeed = 2.5f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Defaults")
	float ViewInterpSpeed = 10.0f;
	
#pragma region Movement

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|State")
	TObjectPtr<const UMovementModelConfig> CurrentMovementConfig;

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|State")
	FCharacterMovementModel MovementModel;

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|State")
	FCharacterMovementState CharMovementState = ECharacterMovementState::None;

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|State")
	ECharacterMovementState PrevMovementState = ECharacterMovementState::None;
	
#pragma endregion Movement

#pragma region Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement|Rotation")
	ECharacterRotationMode InputRotationMode = ECharacterRotationMode::VelocityDirection;

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|Rotation")
	FCharacterRotationMode RotationMode = ECharacterRotationMode::VelocityDirection;
	
#pragma endregion Rotation

#pragma region Stance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement|Stance")
	ECharacterStanceType InputStance = ECharacterStanceType::Standing;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|Stance")
	FCharacterStanceType Stance = ECharacterStanceType::Standing;

#pragma endregion Stance

#pragma region Gait

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement|Gait")
	ECharacterGaitType InputGait = ECharacterGaitType::Run;

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|Gait")
	FCharacterGaitType Gait = ECharacterGaitType::Run;

	UPROPERTY(VisibleInstanceOnly, Category = "Character Movement|Movement")
	ECharacterGaitType AllowedGait = ECharacterGaitType::Walk;

#pragma endregion Gait

	float TargetMaxSpeed = 0.0f;
	
	void CalculateMovement(float DeltaSeconds);
	
	void Sprinting();
	void Crouching();
	void Jumping();

	void OnSprint(bool bEnabled);
	void OnCrouch();
	void OnUnCrouch();
	void OnJump();
	
private:
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
