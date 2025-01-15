// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "ArmorActor.generated.h"

UCLASS()
class STALKER_API AArmorActor : public AItemActor
{
	GENERATED_BODY()

public:
	AArmorActor(const FObjectInitializer& ObjectInitializer);
};
