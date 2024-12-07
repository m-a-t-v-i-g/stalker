// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HandBehaviorConfig.generated.h"

UENUM(BlueprintType)
enum class EMouseButtonReaction : uint8
{
	None,
	Attack,
	Alternative,
	Aiming,
	Power
};

UENUM(BlueprintType)
enum class EOccupiedHand : uint8
{
	Left,
	Right,
	Both
};

USTRUCT(BlueprintType)
struct FHandBehavior
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	EOccupiedHand OccupiedHand = EOccupiedHand::Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FVector InHandOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FRotator InHandRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior",
		meta = (EditCondition = "OccupiedHand == ECharacterSlotHand::Right"))
	EMouseButtonReaction LeftMouseReaction = EMouseButtonReaction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior",
		meta = (EditCondition = "OccupiedHand == ECharacterSlotHand::Right"))
	EMouseButtonReaction RightMouseReaction = EMouseButtonReaction::None;

	void Clear()
	{
		OccupiedHand = EOccupiedHand::Left;
		LeftMouseReaction = EMouseButtonReaction::None;
		RightMouseReaction = EMouseButtonReaction::None;
	}
};

UCLASS()
class STALKER_API UHandBehaviorConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FHandBehavior> ItemsMap;

	const FHandBehavior* GetHandBehavior(const FName& ItemScriptName) const;
};
