// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageSystemCore.generated.h"

UCLASS()
class STALKER_API UDamageSystemCore : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static float CalculateProjectileDamage(float BaseDamage, float DamageMultiplier);
};
