// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemsLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemsFunctionLibrary.generated.h"

class UItemWidget;
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
	static void DestroyItemObject(const UItemObject* ItemObject);

	static UItemObject* GetItemObjectById(const UWorld* World, uint32 ItemId);
	static bool IsItemObjectExist(const UWorld* World, uint32 ItemId);
	
private:
	static void AddItemObjectToGameState(UItemObject* ItemObject);
	static void RemoveItemObjectFromGameState(const UItemObject* ItemObject);
	
	static uint32 LastItemId;
};
