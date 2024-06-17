// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicLibrary.generated.h"

class UCurveVector;

UENUM(BlueprintType)
enum class EOrganicMovementState : uint8
{
	None,
	Ground,
	Airborne,
	Mantle,
	Ragdoll
};

UENUM(BlueprintType)
enum class EOrganicMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
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
	Crouching
};

UENUM(BlueprintType)
enum class EOrganicGait : uint8
{
	Slow,
	Medium,
	Fast
};

USTRUCT(BlueprintType)
struct FMovementState
{
	GENERATED_USTRUCT_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement System")
	EOrganicMovementState State = EOrganicMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement System")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement System")
	bool bGrounded = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement System")
	bool bInAir = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement System")
	bool bMantling = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Organic|Movement System")
	bool bRagdoll = false;

public:
	FMovementState()
	{
	}

	FMovementState(const EOrganicMovementState InitialState) { *this = InitialState; }

	const bool& None() const { return bNone; }
	const bool& Grounded() const { return bGrounded; }
	const bool& InAir() const { return bInAir; }
	const bool& Mantling() const { return bMantling; }
	const bool& Ragdoll() const { return bRagdoll; }

	operator EOrganicMovementState() const { return State; }

	void operator=(const EOrganicMovementState NewState)
	{
		State = NewState;
		bNone = State == EOrganicMovementState::None;
		bGrounded = State == EOrganicMovementState::Ground;
		bInAir = State == EOrganicMovementState::Airborne;
		bMantling = State == EOrganicMovementState::Mantle;
		bRagdoll = State == EOrganicMovementState::Ragdoll;
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

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float DefaultSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float MaxSpeed = 0.0f;

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
		case EOrganicGait::Medium:
			return DefaultSpeed;
		case EOrganicGait::Fast:
			return MaxSpeed;
		case EOrganicGait::Slow:
			return MinSpeed;
		default:
			return DefaultSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FWMovementStanceSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FMovementSettings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMovementStateSettings : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FWMovementStanceSettings ControlDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FWMovementStanceSettings VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FWMovementStanceSettings LookingDirection;
};
