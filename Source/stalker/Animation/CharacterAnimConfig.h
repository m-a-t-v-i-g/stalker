// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OrganicAnimConfig.h"
#include "CharacterAnimConfig.generated.h"

USTRUCT(BlueprintType)
struct FAnimConfig_CharacterConfig
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Character")
	float DynamicTransitionThreshold = 8.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Character")
	float FootHeight = 13.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Character")
	float IK_TraceDistanceAboveFoot = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration|Character")
	float IK_TraceDistanceBelowFoot = 45.0f;
};

UCLASS()
class STALKER_API UCharacterAnimConfig : public UOrganicAnimConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration", meta = (ShowOnlyInnerProperties))
	FAnimConfig_CharacterConfig CharacterConfig;
};
