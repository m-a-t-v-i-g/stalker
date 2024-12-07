// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Organic/BaseOrganic.h"
#include "BaseDog.generated.h"

UCLASS()
class STALKER_API ABaseDog : public ABaseOrganic
{
	GENERATED_BODY()

public:
	ABaseDog(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
