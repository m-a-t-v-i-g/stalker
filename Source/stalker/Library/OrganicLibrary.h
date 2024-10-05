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

#pragma endregion Animation
