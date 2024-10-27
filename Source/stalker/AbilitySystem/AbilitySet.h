// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilitySet.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FAbilitySet_GameplayAbility
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;
};

UCLASS()
class STALKER_API UAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAbilitySet(const FObjectInitializer& ObjectInitializer);

	void GiveToAbilitySystem(class UOrganicAbilityComponent* ASC, UObject* SourceObject = nullptr) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (TitleProperty = Ability))
	TArray<FAbilitySet_GameplayAbility> GrantedGameplayAbilities;
};
