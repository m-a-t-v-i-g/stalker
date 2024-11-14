// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemBehaviorConfig.generated.h"

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
struct FItemBehavior
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	EOccupiedHand OccupiedHand = EOccupiedHand::Right;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "OccupiedHand == ECharacterSlotHand::Right"))
	EMouseButtonReaction LeftMouseReaction = EMouseButtonReaction::Attack;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "OccupiedHand == ECharacterSlotHand::Right"))
	EMouseButtonReaction RightMouseReaction = EMouseButtonReaction::Aiming;
};

UCLASS()
class STALKER_API UItemBehaviorConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FItemBehavior> ItemsMap;

	const FItemBehavior* GetItemBehavior(const FName& ScriptName) const;
};
