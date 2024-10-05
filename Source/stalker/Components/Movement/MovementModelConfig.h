// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicLibrary.h"
#include "Engine/DataAsset.h"
#include "MovementModelConfig.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FMovementModel_Settings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float SlowSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float MediumSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	float FastSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveVector> MovementCurve;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotationRateCurve;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	TObjectPtr<UCurveFloat> RotateLeftCurve;

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
struct FMovementModel_Stance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FMovementModel_Settings Standing;

	UPROPERTY(EditAnywhere, Category = "Movement Settings")
	FMovementModel_Settings Crouching;
};

UCLASS()
class STALKER_API UMovementModelConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Movement Model")
	FMovementModel_Stance ControlDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Model")
	FMovementModel_Stance VelocityDirection;

	UPROPERTY(EditAnywhere, Category = "Movement Model")
	FMovementModel_Stance LookingDirection;
};
