// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

UCLASS()
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

protected:
	TObjectPtr<class UInventoryComponent> InventoryComponent;
	
public:
	AStalkerCharacter();

	virtual void PreInitializeComponents() override;
};
