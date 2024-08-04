// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "WeaponActor.generated.h"

UCLASS()
class STALKER_API AWeaponActor : public AItemActor
{
	GENERATED_BODY()

public:
	AWeaponActor();

protected:
	
public:
	virtual void InitItem(UItemObject* NewItemObject) override;
	
};
