// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPawn.h"
#include "StalkerCharacter.generated.h"

UCLASS()
class STALKER_API AStalkerCharacter : public AGenPawn
{
	GENERATED_BODY()

public:
	AStalkerCharacter();

protected:
	virtual void BeginPlay() override;
};
