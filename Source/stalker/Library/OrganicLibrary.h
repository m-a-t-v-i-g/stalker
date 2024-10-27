// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicLibrary.generated.h"

class UCurveVector;

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	None		UMETA(DisplayName = "None"),
	Ground		UMETA(DisplayName = "Ground"),
	Airborne	UMETA(DisplayName = "Airborne"),
	Ragdoll		UMETA(DisplayName = "Ragdoll")
};

UENUM(BlueprintType)
enum class ECharacterRotationMode : uint8
{
	VelocityDirection	 UMETA(DisplayName = "Velocity"),
	LookingDirection	 UMETA(DisplayName = "Looking"),
	ControlDirection	 UMETA(DisplayName = "Controller")
};

UENUM(BlueprintType)
enum class ECharacterStanceType : uint8
{
	Standing,
	Crouching,
	Crawling
};

UENUM(BlueprintType)
enum class ECharacterGaitType : uint8
{
	Walk,
	Run,
	Sprint
};

UENUM(BlueprintType)
enum class EFootstepType : uint8
{
	Step,
	Walk,
	Run,
	Jump,
	Land
};

UENUM(BlueprintType)
enum class ECharacterMovementDirection : uint8
{
	Forward,
	Right,
	Left,
	Backward
};

UENUM(BlueprintType)
enum class ECharacterHipsDirection : uint8
{
	F	UMETA(DisplayName = "Forward"),
	B	UMETA(DisplayName = "Backward"),
	LF	UMETA(DisplayName = "Left-Forward"),
	LB	UMETA(DisplayName = "Left-Backward"),
	RF	UMETA(DisplayName = "Right-Forward"),
	RB	UMETA(DisplayName = "Right-Backward")
};

UENUM(BlueprintType)
enum class EOrganicFootstepSpawnType : uint8
{
	Location,
	Attached
};

USTRUCT(BlueprintType)
struct FCharacterMovementState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement State")
	ECharacterMovementState State = ECharacterMovementState::None;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement State")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement State")
	bool bGrounded = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement State")
	bool bAirborne = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement State")
	bool bRagdoll = false;

public:
	FCharacterMovementState() {}

	FCharacterMovementState(const ECharacterMovementState InitialState) { *this = InitialState; }

	const bool& None() const { return bNone; }
	const bool& Grounded() const { return bGrounded; }
	const bool& Airborne() const { return bAirborne; }
	const bool& Ragdoll() const { return bRagdoll; }

	operator ECharacterMovementState() const { return State; }

	void operator=(const ECharacterMovementState NewState)
	{
		State = NewState;
		bNone = State == ECharacterMovementState::None;
		bGrounded = State == ECharacterMovementState::Ground;
		bAirborne = State == ECharacterMovementState::Airborne;
		bRagdoll = State == ECharacterMovementState::Ragdoll;
	}
};

USTRUCT(BlueprintType)
struct FCharacterRotationMode
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Rotation Mode")
	ECharacterRotationMode RotationMode = ECharacterRotationMode::VelocityDirection;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Rotation Mode")
	bool bVelocityDirection = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Rotation Mode")
	bool bLookingDirection = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Rotation Mode")
	bool bControlDirection = false;

public:
	FCharacterRotationMode() {}

	FCharacterRotationMode(const ECharacterRotationMode InitialRotationMode) { *this = InitialRotationMode; }

	const bool& VelocityDirection() const { return bVelocityDirection; }
	const bool& LookingDirection() const { return bLookingDirection; }
	const bool& ControlDirection() const { return bControlDirection; }

	operator ECharacterRotationMode() const { return RotationMode; }

	void operator=(const ECharacterRotationMode NewRotationMode)
	{
		RotationMode = NewRotationMode;
		bVelocityDirection = RotationMode == ECharacterRotationMode::VelocityDirection;
		bLookingDirection = RotationMode == ECharacterRotationMode::LookingDirection;
		bControlDirection = RotationMode == ECharacterRotationMode::ControlDirection;
	}
};

USTRUCT(BlueprintType)
struct FCharacterStanceType
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Stance")
	ECharacterStanceType Stance = ECharacterStanceType::Standing;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Stance")
	bool bStanding = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Stance")
	bool bCrouching = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Stance")
	bool bCrawling = false;

public:
	FCharacterStanceType() {}

	FCharacterStanceType(const ECharacterStanceType InitialStance) { *this = InitialStance; }

	const bool& Standing() const { return bStanding; }
	const bool& Crouching() const { return bCrouching; }
	const bool& Crawling() const { return bCrawling; }

	operator ECharacterStanceType() const { return Stance; }

	void operator=(const ECharacterStanceType NewStance)
	{
		Stance = NewStance;
		bStanding = Stance == ECharacterStanceType::Standing;
		bCrouching = Stance == ECharacterStanceType::Crouching;
		bCrawling = Stance == ECharacterStanceType::Crawling;
	}
};

USTRUCT(BlueprintType)
struct FCharacterGaitType
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Gait")
	ECharacterGaitType Gait = ECharacterGaitType::Walk;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Gait")
	bool bWalking = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Gait")
	bool bRunning = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Gait")
	bool bSprinting = false;

public:
	FCharacterGaitType()
	{
	}

	FCharacterGaitType(const ECharacterGaitType InitialGait) { *this = InitialGait; }

	const bool& Walking() const { return bWalking; }
	const bool& Running() const { return bRunning; }
	const bool& Sprinting() const { return bSprinting; }

	operator ECharacterGaitType() const { return Gait; }

	void operator=(const ECharacterGaitType NewGait)
	{
		Gait = NewGait;
		bWalking = Gait == ECharacterGaitType::Walk;
		bRunning = Gait == ECharacterGaitType::Run;
		bSprinting = Gait == ECharacterGaitType::Sprint;
	}
};
