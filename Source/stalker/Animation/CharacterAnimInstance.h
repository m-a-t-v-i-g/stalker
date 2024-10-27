// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAnimConfig.h"
#include "OrganicLibrary.h"
#include "Animation/AnimInstance.h"
#include "Library/CharacterLibrary.h"
#include "CharacterAnimInstance.generated.h"

class UStalkerCharacterMovementComponent;
class UCharacterAnimConfig;
class ABaseCharacter;

USTRUCT(BlueprintType)
struct FCharacterAnim_MovementInfo
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
	ECharacterMovementState PrevMovementState = ECharacterMovementState::None;
};

USTRUCT(BlueprintType)
struct FCharacterAnim_GroundedInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grounded Info")
	ECharacterHipsDirection TrackedHipsDirection = ECharacterHipsDirection::F;

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
struct FCharacterAnim_AirborneInfo
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
struct FCharacterAnim_ViewInfo
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
struct FCharacterAnim_VelocityBlend
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
struct FCharacterAnim_LeanAmount
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lean Amount")
	float FB = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lean Amount")
	float LR = 0.0f;
};

USTRUCT(BlueprintType)
struct FCharacterAnim_LayerBlending
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	int32 OverlayOverrideState = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float EnableAimOffset = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float BasePose_N = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float BasePose_CLF = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L_Add = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L_LS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L_MS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R_Add = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R_LS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R_MS = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Hand_L = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Hand_R = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Legs = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Legs_Add = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Pelvis = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Pelvis_Add = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Spine = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Spine_Add = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Head = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Head_Add = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float EnableHandIK_L = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float EnableHandIK_R = 1.0f;
};

USTRUCT(BlueprintType)
struct FCharacterAnim_FootIK
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	float FootLock_L_Alpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	float FootLock_R_Alpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	bool UseFootLockCurve_L = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	bool UseFootLockCurve_R = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FVector FootLock_L_Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FVector TargetFootLock_R_Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FVector FootLock_R_Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FRotator TargetFootLock_L_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FRotator FootLock_L_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FRotator TargetFootLock_R_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FRotator FootLock_R_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FVector FootOffset_L_Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FVector FootOffset_R_Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FRotator FootOffset_L_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FRotator FootOffset_R_Rotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	FVector PelvisOffset = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
	float PelvisAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FCharacterAnim_DynamicMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Montage")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Montage")
	float BlendInTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Montage")
	float BlendOutTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Montage")
	float PlayRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Montage")
	float StartTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FCharacterAnim_InstanceData
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

USTRUCT(BlueprintType)
struct FCharacterAnim_MovementDirection
{
	GENERATED_USTRUCT_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	ECharacterMovementDirection MovementDirection = ECharacterMovementDirection::Forward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bForward = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bRight = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bLeft = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Movement Direction")
	bool bBackward = false;

public:
	FCharacterAnim_MovementDirection() {}

	FCharacterAnim_MovementDirection(const ECharacterMovementDirection InitialMovementDirection)
	{
		*this = InitialMovementDirection;
	}

	const bool& Forward() const { return bForward; }
	const bool& Backward() const { return bBackward; }
	const bool& Left() const { return bLeft; }
	const bool& Right() const { return bRight; }

	operator ECharacterMovementDirection() const { return MovementDirection; }

	void operator=(const ECharacterMovementDirection NewMovementDirection)
	{
		MovementDirection = NewMovementDirection;
		bForward = MovementDirection == ECharacterMovementDirection::Forward;
		bBackward = MovementDirection == ECharacterMovementDirection::Backward;
		bLeft = MovementDirection == ECharacterMovementDirection::Left;
		bRight = MovementDirection == ECharacterMovementDirection::Right;
	}
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

	static ECharacterMovementDirection CalculateQuadrant(ECharacterMovementDirection Current, float FRThreshold, float FLThreshold,
													   float BRThreshold, float BLThreshold, float Buffer, float Angle);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	TObjectPtr<const UCharacterAnimConfig> AnimConfig;

	UPROPERTY(BlueprintReadOnly, Category = "Input Information")
	TWeakObjectPtr<UStalkerCharacterMovementComponent> CharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Input Information")
	TWeakObjectPtr<ABaseCharacter> Character;

#pragma region Input Info

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Information")
	FCharacterMovementState MovementState = ECharacterMovementState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Information")
	FCharacterRotationMode RotationMode = ECharacterRotationMode::VelocityDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Information")
	FCharacterStanceType Stance = ECharacterStanceType::Standing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Information")
	FCharacterGaitType Gait = ECharacterGaitType::Walk;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Information")
	FCharacterMovementAction MovementAction = ECharacterMovementAction::None;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Information")
	FCharacterOverlayState OverlayState = ECharacterOverlayState::Default;

#pragma endregion Input Info

#pragma region Anim Graph

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_MovementInfo Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_GroundedInfo Grounded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_AirborneInfo Airborne;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_ViewInfo View;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_VelocityBlend VelocityBlend;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_LeanAmount LeanAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_MovementDirection MovementDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_LayerBlending LayerBlending;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_FootIK FootIK;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim Graph")
	FCharacterAnim_InstanceData OrganicAnimData;

#pragma endregion Anim Graph

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
	FCharacterAnim_VelocityBlend CalculateVelocityBlend() const;
	FCharacterAnim_LeanAmount CalculateAirLeanAmount() const;
	ECharacterMovementDirection CalculateMovementDirection() const;

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
	void SetTrackedHipsDirection(ECharacterHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	void DynamicTransitionCheck();
	
	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayTransition(const FCharacterAnim_DynamicMontage& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayTransitionChecked(const FCharacterAnim_DynamicMontage& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Organic|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FCharacterAnim_DynamicMontage Parameters);
	
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
