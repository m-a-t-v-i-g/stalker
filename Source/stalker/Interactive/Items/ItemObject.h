// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

UCLASS(BlueprintType, Blueprintable)
class STALKER_API UItemObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	uint8 SizeX = 1;
	
	UPROPERTY(EditDefaultsOnly)
	uint8 SizeY = 1;

	UPROPERTY(EditDefaultsOnly)
	uint32 ItemId = 0;
};
