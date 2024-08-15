// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "Engine/DataAsset.h"
#include "CharacterItemsDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FItemBehavior
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	ECharacterOverlayState OverlayState = ECharacterOverlayState::Default;

	/** How big is this item? */
	UPROPERTY(EditAnywhere)
	bool bTwoHanded = true;

	/**
	 * Primary item has own overlay state and will be armed at right hand. Another hasn't state and arming at left hand.
	 */
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bTwoHanded"))
	bool bPrimary = true;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bTwoHanded && bPrimary"))
	ECharacterOverlayState OneHandOverlay = ECharacterOverlayState::Default;
};

UCLASS()
class STALKER_API UCharacterItemsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	TMap<FName, FItemBehavior> ItemsMap;
};
