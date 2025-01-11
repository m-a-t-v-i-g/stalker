// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterAnimConfig.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FAnimConfig_TurnInPlaceAsset
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place Asset")
	TObjectPtr<UAnimSequenceBase> Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place Asset")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place Asset")
	float AnimatedAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place Asset")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place Asset")
	bool ScaleTurnAngle = true;
};

USTRUCT(BlueprintType)
struct FAnimConfig_TurnInPlace
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float TurnCheckMinAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float Turn180Threshold = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float AimYawRateLimit = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float MinAngleDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	float MaxAngleDelay = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset N_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset N_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset N_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset N_TurnIP_R180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset CLF_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset CLF_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset CLF_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turn In Place")
	FAnimConfig_TurnInPlaceAsset CLF_TurnIP_R180;
};

USTRUCT(BlueprintType)
struct FAnimConfig_RotateInPlace
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate In Place")
	float RotateMinThreshold = -50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate In Place")
	float RotateMaxThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate In Place")
	float ViewYawRateMinRange = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate In Place")
	float ViewYawRateMaxRange = 270.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate In Place")
	float MinPlayRate = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotate In Place")
	float MaxPlayRate = 3.0f;
};

USTRUCT(BlueprintType)
struct FAnimConfig_CharacterConfig
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float AnimatedWalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float AnimatedRunSpeed = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float AnimatedSprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float AnimatedCrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float VelocityBlendInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float GroundedLeanInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float SmoothedAimingRotationInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float InputYawOffsetInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float TriggerPivotSpeedLimit = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float DynamicTransitionThreshold = 8.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float FootHeight = 13.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float IK_TraceDistanceAboveFoot = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	float IK_TraceDistanceBelowFoot = 45.0f;
	
};

UCLASS()
class STALKER_API UCharacterAnimConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration", meta = (ShowOnlyInnerProperties))
	FAnimConfig_CharacterConfig CharacterConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FAnimConfig_TurnInPlace TurnInPlace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration")
	FAnimConfig_RotateInPlace RotateInPlace;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScale;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LandPrediction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LeanAirborne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_FB;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_LR;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk;
};
