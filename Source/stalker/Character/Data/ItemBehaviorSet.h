// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemBehaviorSet.generated.h"

class UAbilitySet;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	TObjectPtr<const UAbilitySet> AbilitySet;

	void Clear()
	{
		OccupiedHand = EOccupiedHand::Left;
		InHandOffset = FVector::ZeroVector;
		InHandRotation = FRotator::ZeroRotator;
		AbilitySet = nullptr;
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
class STALKER_API UItemBehaviorSet : public UPrimaryDataAsset
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
