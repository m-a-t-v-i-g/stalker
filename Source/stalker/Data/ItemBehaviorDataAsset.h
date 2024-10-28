// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "Engine/DataAsset.h"
#include "ItemBehaviorDataAsset.generated.h"

UENUM(BlueprintType)
enum class EMouseButtonReaction : uint8
{
	Attack,
	Alternative,
	Aiming,
	Power
};

UENUM(BlueprintType)
enum class ECharacterSlotHand : uint8
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
	EMouseButtonReaction LeftMouseReaction = EMouseButtonReaction::Attack;
	
	UPROPERTY(EditAnywhere)
	EMouseButtonReaction RightMouseReaction = EMouseButtonReaction::Alternative;
	
	UPROPERTY(EditAnywhere)
	ECharacterSlotHand OccupiedHand = ECharacterSlotHand::Right;
	
	UPROPERTY(EditAnywhere)
	ECharacterOverlayState PrimaryOverlay = ECharacterOverlayState::Default;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "OccupiedHand == ECharacterSlotHand::Right"))
	ECharacterOverlayState SecondaryOverlay = ECharacterOverlayState::Default;
};

UCLASS()
class STALKER_API UItemBehaviorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FItemBehavior> ItemsMap;
};
