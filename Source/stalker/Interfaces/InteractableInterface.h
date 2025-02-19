﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE()
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class STALKER_API IInteractableInterface
{
	GENERATED_BODY()

public:
	virtual bool OnInteract(AActor* Interactor);
};
