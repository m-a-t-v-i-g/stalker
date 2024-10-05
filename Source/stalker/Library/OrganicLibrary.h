// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicLibrary.generated.h"

class UCurveVector;

UENUM(BlueprintType)
enum class EOrganicMovementState : uint8
{
	None		UMETA(DisplayName = "None"),
	Ground		UMETA(DisplayName = "Ground"),
	Airborne	UMETA(DisplayName = "Airborne"),
	Ragdoll		UMETA(DisplayName = "Ragdoll")
};

UENUM(BlueprintType)
enum class EOrganicRotationMode : uint8
{
	VelocityDirection,
	LookingDirection,
	ControlDirection
};

UENUM(BlueprintType)
enum class EOrganicStance : uint8
{
	Standing,
	Crouching,
	Crawling
};

UENUM(BlueprintType)
enum class EOrganicGait : uint8
{
	Slow,
	Medium,
	Fast
};

UENUM(BlueprintType)
enum class EOrganicFootstepType : uint8
{
	Step,
	Walk,
	Run,
	Jump,
	Land
};

UENUM(BlueprintType)
enum class EOrganicMovementDirection : uint8
{
	Forward,
	Right,
	Left,
	Backward
};

UENUM(BlueprintType)
enum class EOrganicHipsDirection : uint8
{
	F,
	B,
	LF,
	LB,
	RF,
	RB
};

UENUM(BlueprintType)
enum class EOrganicFootstepSpawnType : uint8
{
	Location,
	Attached
};

USTRUCT(BlueprintType)
struct FOrganicMovementState
{
	GENERATED_USTRUCT_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement State")
	EOrganicMovementState State = EOrganicMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement State")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement State")
	bool bGrounded = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement State")
	bool bAirborne = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement State")
	bool bRagdoll = false;

public:
	FOrganicMovementState()
	{
	}

	FOrganicMovementState(const EOrganicMovementState InitialState) { *this = InitialState; }

	const bool& None() const { return bNone; }
	const bool& Grounded() const { return bGrounded; }
	const bool& Airborne() const { return bAirborne; }
	const bool& Ragdoll() const { return bRagdoll; }

	operator EOrganicMovementState() const { return State; }

	void operator=(const EOrganicMovementState NewState)
	{
		State = NewState;
		bNone = State == EOrganicMovementState::None;
		bGrounded = State == EOrganicMovementState::Ground;
		bAirborne = State == EOrganicMovementState::Airborne;
		bRagdoll = State == EOrganicMovementState::Ragdoll;
	}
};

USTRUCT(BlueprintType)
struct FOrganicMovementDirection
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	EOrganicMovementDirection MovementDirection = EOrganicMovementDirection::Forward;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bForward = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bRight = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bLeft = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "ALS|Movement System")
	bool bBackward = false;

public:
	FOrganicMovementDirection()
	{
	}

	FOrganicMovementDirection(const EOrganicMovementDirection InitialMovementDirection)
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
struct FOrganicRotationMode
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Rotation System")
	EOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Rotation System")
	bool bVelocityDirection = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Rotation System")
	bool bLookingDirection = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Rotation System")
	bool bControlDirection = false;

public:
	FOrganicRotationMode()
	{
	}

	FOrganicRotationMode(const EOrganicRotationMode InitialRotationMode) { *this = InitialRotationMode; }

	const bool& VelocityDirection() const { return bVelocityDirection; }
	const bool& LookingDirection() const { return bLookingDirection; }
	const bool& ControlDirection() const { return bControlDirection; }

	operator EOrganicRotationMode() const { return RotationMode; }

	void operator=(const EOrganicRotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		bVelocityDirection = RotationMode == EOrganicRotationMode::VelocityDirection;
		bLookingDirection = RotationMode == EOrganicRotationMode::LookingDirection;
		bControlDirection = RotationMode == EOrganicRotationMode::ControlDirection;
	}
};

USTRUCT(BlueprintType)
struct FOrganicStance
{
	GENERATED_USTRUCT_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Stance")
	EOrganicStance Stance = EOrganicStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Stance")
	bool bStanding = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Stance")
	bool bCrouching = false;

public:
	FOrganicStance()
	{
	}

	FOrganicStance(const EOrganicStance InitialStance) { *this = InitialStance; }

	const bool& Standing() const { return bStanding; }
	const bool& Crouching() const { return bCrouching; }

	operator EOrganicStance() const { return Stance; }

	void operator=(const EOrganicStance NewStance)
	{
		Stance = NewStance;
		bStanding = Stance == EOrganicStance::Standing;
		bCrouching = Stance == EOrganicStance::Crouching;
	}
};

USTRUCT(BlueprintType)
struct FOrganicGait
{
	GENERATED_USTRUCT_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Gait")
	EOrganicGait Gait = EOrganicGait::Slow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Gait")
	bool bWalking = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Gait")
	bool bRunning = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Gait")
	bool bSprinting = false;

public:
	FOrganicGait()
	{
	}

	FOrganicGait(const EOrganicGait InitialGait) { *this = InitialGait; }

	const bool& Walking() const { return bWalking; }
	const bool& Running() const { return bRunning; }
	const bool& Sprinting() const { return bSprinting; }

	operator EOrganicGait() const { return Gait; }

	void operator=(const EOrganicGait NewGait)
	{
		Gait = NewGait;
		bWalking = Gait == EOrganicGait::Slow;
		bRunning = Gait == EOrganicGait::Medium;
		bSprinting = Gait == EOrganicGait::Fast;
	}
};

#pragma region Movement Settings

USTRUCT(BlueprintType)
struct FOrganicMovementSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SlowSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float MediumSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float FastSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve = nullptr;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotateLeftCurve = nullptr;

	float GetSpeedForGait(const EOrganicGait Gait) const
	{
		switch (Gait)
		{
		case EOrganicGait::Slow:
			return SlowSpeed;
		case EOrganicGait::Medium:
			return MediumSpeed;
		case EOrganicGait::Fast:
			return FastSpeed;
		default:
			return MediumSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FOrganicMovementStances
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FOrganicMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FOrganicMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FOrganicMovementModel : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FOrganicMovementStances ControlDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FOrganicMovementStances VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FOrganicMovementStances LookingDirection;
};

#pragma endregion Movement Settings

#pragma region Animation

USTRUCT(BlueprintType)
struct FOrganicDynamicMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organic|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organic|Dynamic Transition")
	float BlendInTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organic|Dynamic Transition")
	float BlendOutTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organic|Dynamic Transition")
	float PlayRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organic|Dynamic Transition")
	float StartTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicLeanAmount
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float FB = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float LR = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicVelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float Forward = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float Backward = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float Left = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Grounded")
	float Right = 0.0f;
};

USTRUCT(BlueprintType)
struct FOrganicTurnInPlaceAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Turn In Place")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Turn In Place")
	float AnimatedAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Turn In Place")
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Turn In Place")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Turn In Place")
	bool ScaleTurnAngle = true;
};

USTRUCT(BlueprintType)
struct FAnimOrganicGrounded
{
	GENERATED_BODY()

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
	float DiagonalScaleAmount = 0.0f;

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
struct FAnimOrganicAirborne
{
	GENERATED_BODY()

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
struct FAnimOrganicViewValues
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	FRotator SmoothedViewRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	FRotator SpineRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|View Values")
	FVector2D AimingAngle;

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
struct FAnimOrganicLayerBlending
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	int32 OverlayOverrideState = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float EnableAimOffset = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float BasePose_N = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float BasePose_CLF = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L_LS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_L_MS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R_LS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Arm_R_MS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Hand_L = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Hand_R = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Legs = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Legs_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Pelvis = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Pelvis_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Spine = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Spine_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Head = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float Head_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float EnableHandIK_L = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Layer Blending")
	float EnableHandIK_R = 1.0f;
};

USTRUCT(BlueprintType)
struct FAnimOrganicFootIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	float FootLock_L_Alpha = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	float FootLock_R_Alpha = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	bool UseFootLockCurve_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	bool UseFootLockCurve_R;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FVector FootLock_L_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FVector TargetFootLock_R_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FVector FootLock_R_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FRotator TargetFootLock_L_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FRotator FootLock_L_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FRotator TargetFootLock_R_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FRotator FootLock_R_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FVector FootOffset_L_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FVector FootOffset_R_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FRotator FootOffset_L_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FRotator FootOffset_R_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	FVector PelvisOffset;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Organic|Anim Graph|Foot IK")
	float PelvisAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnimOrganicTurnInPlace
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	float TurnCheckMinAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	float Turn180Threshold = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	float AimYawRateLimit = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	float ElapsedDelayTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	float MinAngleDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	float MaxAngleDelay = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset N_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset N_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset N_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset N_TurnIP_R180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset CLF_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset CLF_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset CLF_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Graph|Turn In Place")
	FOrganicTurnInPlaceAsset CLF_TurnIP_R180;
};

USTRUCT(BlueprintType)
struct FAnimOrganicRotateInPlace
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Rotate In Place")
	float RotateMinThreshold = -50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Rotate In Place")
	float RotateMaxThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Rotate In Place")
	float ViewYawRateMinRange = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Rotate In Place")
	float ViewYawRateMaxRange = 270.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Rotate In Place")
	float MinPlayRate = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Rotate In Place")
	float MaxPlayRate = 3.0f;
};

USTRUCT(BlueprintType)
struct FAnimOrganicConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float AnimatedWalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float AnimatedRunSpeed = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float AnimatedSprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float AnimatedCrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float VelocityBlendInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float GroundedLeanInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float AirborneLeanInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float SmoothedAimingRotationInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float InputYawOffsetInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float TriggerPivotSpeedLimit = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float FootHeight = 13.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float DynamicTransitionThreshold = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float IK_TraceDistanceAboveFoot = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic|Anim Configuration")
	float IK_TraceDistanceBelowFoot = 45.0f;
};

#pragma endregion Animation
