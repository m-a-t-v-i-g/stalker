// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemSystemCore.generated.h"

class UItemDefinition;
class UItemPredictedData;
class UItemObject;

UCLASS()
class STALKER_API UItemSystemCore : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UItemObject* GenerateItemObject(UWorld* World, const UItemObject* ItemObject);
	static UItemObject* GenerateItemObject(UWorld* World, const UItemDefinition* Definition,
	                                       const UItemPredictedData* PredictedData);
	static void DestroyItemObject(const UItemObject* ItemObject);

	static UItemObject* GetItemObjectById(const UWorld* World, uint32 ItemId);
	static bool IsItemObjectExist(const UWorld* World, uint32 ItemId);
	
private:
	static void AddItemObjectToGameState(UItemObject* ItemObject);
	static void RemoveItemObjectFromGameState(const UItemObject* ItemObject);
	
	static uint32 LastItemId;
};
