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

UENUM(BlueprintType)
enum class EArmorType : uint8
{
	Helmet,
	Body
};

USTRUCT(BlueprintType)
struct FWeaponBehavior
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	EOccupiedHand OccupiedHand = EOccupiedHand::Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FVector InHandOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	FRotator InHandRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior",
		meta = (EditCondition = "OccupiedHand != EOccupiedHand::Left"))
	EMouseButtonReaction LeftMouseReaction = EMouseButtonReaction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior",
		meta = (EditCondition = "OccupiedHand != EOccupiedHand::Left"))
	EMouseButtonReaction RightMouseReaction = EMouseButtonReaction::None;

	void Clear()
	{
		OccupiedHand = EOccupiedHand::Left;
		LeftMouseReaction = EMouseButtonReaction::None;
		RightMouseReaction = EMouseButtonReaction::None;
	}
};

USTRUCT(BlueprintType)
struct FArmorBehavior
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	EArmorType ArmorType = EArmorType::Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	TObjectPtr<USkeletalMesh> CharacterMesh;

	void Clear()
	{
		ArmorType = EArmorType::Body;
		CharacterMesh = nullptr;
	}
};

UCLASS()
class STALKER_API UItemBehaviorConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FWeaponBehavior> Weapons;

	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FArmorBehavior> Armors;

	const FWeaponBehavior* GetWeaponBehavior(const FName& ItemScriptName) const;
	const FArmorBehavior* GetArmorBehavior(const FName& ItemScriptName) const;
};
