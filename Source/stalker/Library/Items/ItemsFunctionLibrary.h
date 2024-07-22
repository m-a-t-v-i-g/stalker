// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemsLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemsFunctionLibrary.generated.h"

UCLASS()
class STALKER_API UItemsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UItemObject* GenerateItemObject(const FItemData& ItemData);

private:
	static uint32 LastItemId;
};
