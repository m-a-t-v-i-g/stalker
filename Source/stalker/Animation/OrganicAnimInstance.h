// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Library/OrganicLibrary.h"
#include "OrganicAnimInstance.generated.h"

class UAnimSequence;
class UCurveFloat;
class UCurveVector;
class UOrganicMovementComponent;

USTRUCT(BlueprintType)
struct FAnim_MovementInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FVector RelativeVelocityDirection = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	FRotator ViewRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float ViewYawRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	float ZoomAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement Info")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;
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

	virtual void OnLayerValuesUpdated(float DeltaSeconds);
	
	virtual void UpdateViewValues(float DeltaSeconds);
	virtual void UpdateLayerValues(float DeltaSeconds);
	virtual void UpdateFootIK(float DeltaSeconds);

	void UpdateMovementValues(float DeltaSeconds);
	void UpdateRotationValues();
	
	virtual void UpdateGroundedValues(float DeltaSeconds);
	virtual void UpdateAirborneValues(float DeltaSeconds);
	virtual void UpdateRagdollValues();

	virtual void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	                    float& CurFootLockAlpha, bool& UseFootLockCurve,
	                    FVector& CurFootLockLoc, FRotator& CurFootLockRot);
	virtual void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot);
	virtual void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);
	virtual void ResetIKOffsets(float DeltaSeconds);
	virtual void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	                    FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset);

	void RotateInPlaceCheck();
	void TurnInPlaceCheck(float DeltaSeconds);

	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	float CalculateStrideBlend() const;
	float CalculateWalkRunBlend() const;
	float CalculateStandingPlayRate() const;
	float CalculateDiagonalScaleAmount() const;
	float CalculateCrouchingPlayRate() const;
	float CalculateLandPrediction() const;

	FVector CalculateRelativeAccelerationAmount() const;

	FOrganicVelocityBlend CalculateVelocityBlend() const;
	FOrganicLeanAmount CalculateAirLeanAmount() const;
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
	TWeakObjectPtr<UOrganicMovementComponent> OrganicMovement;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information",
		meta = (ShowOnlyInnerProperties))
	FAnim_MovementInfo MovementInfo;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicMovementState MovementState = EOrganicMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicStance Stance = EOrganicStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic Information")
	FOrganicGait Gait = EOrganicGait::Slow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded",
		meta = (ShowOnlyInnerProperties))
	FAnimOrganicGrounded Grounded;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Airborne",
		meta = (ShowOnlyInnerProperties))
	FAnimOrganicAirborne Airborne;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - View Values",
		meta = (ShowOnlyInnerProperties))
	FAnimOrganicViewValues ViewValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrganicVelocityBlend VelocityBlend;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrganicLeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FOrganicMovementDirection MovementDirection = EOrganicMovementDirection::Forward;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - View Values")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Ragdoll")
	float FlailRate = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (
		ShowOnlyInnerProperties))
	FAnimOrganicLayerBlending LayerBlendingValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FAnimOrganicFootIK FootIKValues;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Turn In Place", Meta = (
		ShowOnlyInnerProperties))
	FAnimOrganicTurnInPlace TurnInPlaceValues;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Place", Meta = (
		ShowOnlyInnerProperties))
	FAnimOrganicRotateInPlace RotateInPlace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Main Configuration", Meta = (
		ShowOnlyInnerProperties))
	FAnimOrganicConfiguration Config;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LandPredictionCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LeanInAirCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_FB = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_LR = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L = nullptr;

private:
	FTimerHandle OnJumpedTimer;
	FTimerHandle OnPivotTimer;

protected:
	static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);

	static EOrganicMovementDirection CalculateQuadrant(EOrganicMovementDirection Current, float FRThreshold,
	                                                   float FLThreshold,
	                                                   float BRThreshold, float BLThreshold, float Buffer, float Angle);
};
