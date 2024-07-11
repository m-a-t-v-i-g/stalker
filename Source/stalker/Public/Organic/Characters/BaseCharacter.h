// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Organic/BaseOrganic.h"
#include "BaseCharacter.generated.h"

UCLASS()
class STALKER_API ABaseCharacter : public ABaseOrganic
{
	GENERATED_BODY()

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
