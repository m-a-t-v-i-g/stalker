// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArmorBehaviorConfig.generated.h"

UENUM(BlueprintType)
enum class EArmorType : uint8
{
	Helmet,
	Body
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
class STALKER_API UArmorBehaviorConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FArmorBehavior> ItemsMap;

	const FArmorBehavior* GetHandBehavior(const FName& ItemScriptName) const;
};
