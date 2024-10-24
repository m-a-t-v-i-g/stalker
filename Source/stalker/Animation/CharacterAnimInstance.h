// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAnimConfig.h"
#include "OrganicLibrary.h"
#include "Animation/AnimInstance.h"
#include "Library/CharacterLibrary.h"
#include "CharacterAnimInstance.generated.h"

class UOrganicMovementComponent;
class UCharacterAnimConfig;
class ABaseCharacter;

USTRUCT(BlueprintType)
struct FOrganicAnim_MovementInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	FRotator ActorRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	FRotator ViewRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	float ViewYawRate = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	float MovementInputAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	bool bHasMovementInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Info")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_GroundedInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grounded Info")
	EOrganicHipsDirection TrackedHipsDirection = EOrganicHipsDirection::F;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	bool bShouldMove = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	bool bRotateL = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	bool bRotateR = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grounded Info")
	bool bPivot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float RotateRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float RotationScale = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float DiagonalScale = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float WalkRunBlend = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float StandingPlayRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float CrouchingPlayRate = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float StrideBlend = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float FYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float BYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float LYaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grounded Info")
	float RYaw = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_AirborneInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airborne Info")
	bool bJumped = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airborne Info")
	float JumpPlayRate = 1.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airborne Info")
	float FallSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airborne Info")
	float LandPrediction = 1.0f;
};

USTRUCT(BlueprintType)
struct FOrganicAnim_ViewInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	FRotator SmoothedViewRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	FRotator SpineRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	FVector2D AimOffset = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	FVector2D AimingAngle = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	float AimSweepTime = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	float InputYawOffsetTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	float ForwardYawTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
	float LeftYawTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View Info")
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
	GENERATED_USTRUCT_BODY()

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	EOrganicMovementDirection MovementDirection = EOrganicMovementDirection::Forward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bForward = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bRight = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bLeft = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
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
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Moving")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "View")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ragdoll")
	float FlailRate = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float ElapsedDelayTime = 0.0f;
};

UCLASS()
class STALKER_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	template<class T>
	T* GetAnimConfig()
	{
		return CastChecked<T>(AnimConfig);
	}
	
	static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer);

	static EOrganicMovementDirection CalculateQuadrant(EOrganicMovementDirection Current, float FRThreshold, float FLThreshold,
													   float BRThreshold, float BLThreshold, float Buffer, float Angle);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TObjectPtr<const UCharacterAnimConfig> AnimConfig;

	UPROPERTY(BlueprintReadOnly, Category = "Input Information")
	TWeakObjectPtr<UOrganicMovementComponent> OrganicMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Input Information")
	TWeakObjectPtr<ABaseCharacter> Character;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FOrganicMovementState MovementState = EOrganicMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FOrganicStance Stance = EOrganicStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FOrganicGait Gait = EOrganicGait::Walk;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FCharacterMovementAction MovementAction = ECharacterMovementAction::None;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Input Information")
	FCharacterOverlayState OverlayState = ECharacterOverlayState::Default;

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
	FAnimOrganicLayerBlending LayerBlending;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Anim Graph")
	FAnimOrganicFootIK FootIK;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Anim Graph")
	FOrganicAnim_InstanceData OrganicAnimData;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void UpdateMovementInfo(float DeltaSeconds);
	virtual void UpdateViewInfo(float DeltaSeconds);
	
	virtual void UpdateMovementValues(float DeltaSeconds);
	virtual void UpdateRotationValues(float DeltaSeconds);
	virtual void UpdateLayerValues(float DeltaSeconds);
	virtual void UpdateFootIK(float DeltaSeconds);

	virtual void UpdateGroundedValues(float DeltaSeconds);
	virtual void UpdateAirborneValues(float DeltaSeconds);
	virtual void UpdateRagdollValues(float DeltaSeconds);

	void RotateInPlaceCheck();
	void TurnInPlaceCheck(float DeltaSeconds);
	void TurnInPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);
	FAnimConfig_TurnInPlaceAsset GetTurnInPlaceAsset(float TurnAngle) const;

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
	
	virtual void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	                            float& CurFootLockAlpha, bool& UseFootLockCurve, FVector& CurFootLockLoc,
	                            FRotator& CurFootLockRot);
	virtual void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot);
	virtual void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);
	virtual void ResetIKOffsets(float DeltaSeconds);
	virtual void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	                            FVector& CurLocationTarget, FVector& CurLocationOffset,
	                            FRotator& CurRotationOffset);

	UFUNCTION(BlueprintCallable, Category = "Organic|Grounded")
	void SetTrackedHipsDirection(EOrganicHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	void DynamicTransitionCheck();
	
	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayTransition(const FOrganicDynamicMontage& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayTransitionChecked(const FOrganicDynamicMontage& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FOrganicDynamicMontage Parameters);
	
	void PlayDynamicTransitionDelay();

	UFUNCTION(BlueprintCallable, Category = "Organic|Grounded")
	bool CanDynamicTransition() const;
	
	FCollisionShape MakeCollisionShape() const;
	
private:
	FCollisionShape CollisionShape;
	
	FTimerHandle OnJumpedTimer;
	FTimerHandle OnPivotTimer;
	FTimerHandle PlayDynamicTransitionTimer;

	bool bCanPlayDynamicTransition = true;
};
