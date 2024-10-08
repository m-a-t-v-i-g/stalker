// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "OrganicAnimConfig.h"
#include "Animation/AnimInstance.h"
#include "Library/OrganicLibrary.h"
#include "OrganicAnimInstance.generated.h"

class UAnimSequence;
class UCurveFloat;
class UCurveVector;
class UOrganicMovementComponent;

USTRUCT(BlueprintType)
struct FOrganicAnim_MovementInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FRotator ActorRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FRotator ViewRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float ViewYawRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_GroundedInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Organic|Anim Graph|Grounded")
	EOrganicHipsDirection TrackedHipsDirection = EOrganicHipsDirection::F;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	bool bShouldMove = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	bool bRotateL = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	bool bRotateR = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Organic|Anim Graph|Grounded")
	bool bPivot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float RotateRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float RotationScale = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float DiagonalScale = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float WalkRunBlend = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float StandingPlayRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float CrouchingPlayRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float StrideBlend = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float FYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float BYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float LYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float RYaw = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_AirborneInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Airborne")
	bool bJumped = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Airborne")
	float JumpPlayRate = 1.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Airborne")
	float FallSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Airborne")
	float LandPrediction = 1.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_ViewInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	FRotator SmoothedViewRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	FRotator SpineRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	FVector2D AimingAngle = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	float AimSweepTime = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	float InputYawOffsetTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	float ForwardYawTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	float LeftYawTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	float RightYawTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_VelocityBlend
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Blend")
	float Forward = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Blend")
	float Backward = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Blend")
	float Left = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Velocity Blend")
	float Right = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_LeanAmount
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lean Amount")
	float FB = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lean Amount")
	float LR = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_MovementDirection
{
	GENERATED_USTRUCT_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	EOrganicMovementDirection MovementDirection = EOrganicMovementDirection::Forward;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bForward = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bRight = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bLeft = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bBackward = false;

public:
	FOrganicAnim_MovementDirection() {}

	FOrganicAnim_MovementDirection(const EOrganicMovementDirection InitialMovementDirection)
	{
		*this = InitialMovementDirection;
	}

	const bool& Forward() const { return bForward; }
	const bool& Backward() const { return bBackward; }
	const bool& Left() const { return bLeft; }
	const bool& Right() const { return bRight; }

	operator EOrganicMovementDirection() const { return MovementDirection; }

	void operator=(const EOrganicMovementDirection NewMovementDirection)
	{
		MovementDirection = NewMovementDirection;
		bForward = MovementDirection == EOrganicMovementDirection::Forward;
		bBackward = MovementDirection == EOrganicMovementDirection::Backward;
		bLeft = MovementDirection == EOrganicMovementDirection::Left;
		bRight = MovementDirection == EOrganicMovementDirection::Right;
	}
};

USTRUCT(BlueprintType)
struct FOrganicAnim_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float ElapsedDelayTime = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class STALKER_API UOrganicAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Organic|Grounded")
	void SetTrackedHipsDirection(EOrganicHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	virtual void UpdateMovementInfo(float DeltaSeconds);
	virtual void UpdateViewValues(float DeltaSeconds);

	void UpdateMovementValues(float DeltaSeconds);
	void UpdateRotationValues();
	
	virtual void UpdateGroundedValues(float DeltaSeconds);
	virtual void UpdateAirborneValues(float DeltaSeconds);
	virtual void UpdateRagdollValues();

	void RotateInPlaceCheck();
	
	void TurnInPlaceCheck(float DeltaSeconds);
	void TurnInPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	virtual FAnimConfig_TurnInPlaceAsset GetTurnInPlaceAsset(float TurnAngle) const;

	float CalculateStrideBlend() const;
	float CalculateGaitBlend() const;
	float CalculateStandingPlayRate() const;
	float CalculateDiagonalScale() const;
	float CalculateCrouchingPlayRate() const;
	float CalculateLandPrediction() const;

	FVector CalculateRelativeAccelerationAmount() const;

	FOrganicAnim_VelocityBlend CalculateVelocityBlend() const;
	FOrganicAnim_LeanAmount CalculateAirLeanAmount() const;
	
	EOrganicMovementDirection CalculateMovementDirection() const;

	float GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const;

	bool ShouldMoveCheck() const;

	bool CanRotateInPlace() const;
	bool CanTurnInPlace() const;

	UFUNCTION(BlueprintCallable, Category = "Organic|Event")
	void OnJumped();

	UFUNCTION(BlueprintCallable, Category = "Organic|Event")
	void OnPivot();

	void OnJumpedDelay();
	void OnPivotDelay();

public:
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_BasePose_N = "BasePose_N";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_BasePose_CLF = "BasePose_CLF";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Mask_AimOffset = "Mask_AimOffset";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Mask_LandPrediction = "Mask_LandPrediction";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Grounded_Slot = "Grounded Slot";

	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_Enable_Transition = "Enable_Transition";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME_W_Gait = "W_Gait";
	
	UPROPERTY(EditDefaultsOnly, Category = "Configuration|Blend Curves")
	FName NAME__ALSCharacterAnimInstance__root = "root";
	
	UPROPERTY(BlueprintReadOnly, Category = "Organic Information")
	TObjectPtr<class ABaseOrganic> OrganicPawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Organic Information")
	TWeakObjectPtr<const UOrganicMovementComponent> OrganicMovement;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicMovementState MovementState = EOrganicMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicStance Stance = EOrganicStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicGait Gait = EOrganicGait::Slow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_MovementInfo Movement;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_GroundedInfo Grounded;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_AirborneInfo Airborne;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_ViewInfo View;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_VelocityBlend VelocityBlend;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_LeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_MovementDirection MovementDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_InstanceData OrganicAnimData;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	float FlailRate = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TObjectPtr<const UOrganicAnimConfig> AnimConfig;

private:
	FTimerHandle OnJumpedTimer;
	FTimerHandle OnPivotTimer;

public:
	template<class T>
	T* GetAnimConfig()
	{
		return CastChecked<T>(AnimConfig);
	}
	
protected:
	static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);

	static EOrganicMovementDirection CalculateQuadrant(EOrganicMovementDirection Current, float FRThreshold, float FLThreshold,
	                                                   float BRThreshold, float BLThreshold, float Buffer, float Angle);
};
