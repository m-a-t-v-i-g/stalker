// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicLibrary.h"
#include "Engine/DataAsset.h"
#include "MovementModelConfig.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FCharacterMovementModel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float WalkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float RunSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SprintSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationCurve;

	float GetSpeedForGait(const ECharacterGaitType Gait) const
	{
		switch (Gait)
		{
		case ECharacterGaitType::Walk:
			return WalkSpeed;
		case ECharacterGaitType::Run:
			return RunSpeed;
		case ECharacterGaitType::Sprint:
			return SprintSpeed;
		default:
			return RunSpeed;
		}
	}
};

USTRUCT(BlueprintType)
struct FCharacterMovementSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FCharacterMovementModel Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FCharacterMovementModel Crouching;
};

UCLASS()
class STALKER_API UMovementModelConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Movement Model")
	FCharacterMovementSettings VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Model")
	FCharacterMovementSettings LookingDirection;
	
	UPROPERTY(EditAnywhere, Category = "Movement Model")
	FCharacterMovementSettings ControlDirection;

	FCharacterMovementModel GetMovementSettings(const ECharacterRotationMode RotationMode,
	                                            const ECharacterStanceType Stance) const;
};
