// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UsableInterface.generated.h"

UINTERFACE()
class UUsableInterface : public UInterface
{
	GENERATED_BODY()
};

class STALKER_API IUsableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Use(UObject* Source);
};
