// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Movement/StalkerCharacterMovementComponent.h"
#include "StalkerCharMovementComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UStalkerCharMovementComponent : public UStalkerCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UStalkerCharMovementComponent();

	virtual void BindReplicationData_Implementation() override;
};
