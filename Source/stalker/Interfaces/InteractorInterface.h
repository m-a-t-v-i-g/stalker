// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractorInterface.generated.h"

class UItemObject;

UINTERFACE()
class UInteractorInterface : public UInterface
{
	GENERATED_BODY()
};

class STALKER_API IInteractorInterface
{
	GENERATED_BODY()

public:
	virtual bool ContainerInteract();

	virtual bool ItemInteract(const UItemObject* ItemObject);
};
