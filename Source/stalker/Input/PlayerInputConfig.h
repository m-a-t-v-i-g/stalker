// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PlayerInputConfig.generated.h"

class UInputAction;

USTRUCT()
struct FPlayerInputInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Input Config")
	TObjectPtr<const UInputAction> InputAction;
 
	UPROPERTY(EditDefaultsOnly, Category = "Input Config")
	FGameplayTag InputTag;
};

UCLASS()
class STALKER_API UPlayerInputConfig : public UPrimaryDataAsset
{
	GENERATED_UCLASS_BODY()

	const UInputAction* FindNativeAction(const FGameplayTag& InputTag) const;
	const UInputAction* FindAbilityAction(const FGameplayTag& InputTag) const;

	UPROPERTY(EditAnywhere, Category = "Inputs")
	TArray<FPlayerInputInfo> NativeActions;
 
	UPROPERTY(EditAnywhere, Category = "Inputs")
	TArray<FPlayerInputInfo> AbilityActions;
};