// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemsLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemsFunctionLibrary.generated.h"

class UItemDefinition;
class UItemPredictedData;

UCLASS()
class STALKER_API UItemsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UItemObject* GenerateItemObject(UWorld* World, UItemObject* ItemObject);
	static UItemObject* GenerateItemObject(UWorld* World, const UItemDefinition* Definition,
	                                       const UItemPredictedData* PredictedData);

private:
	static void AddItemObjectToGameState(UItemObject* ItemObject);
	
	static uint32 LastItemId;
};
