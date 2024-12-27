// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponActor.h"
#include "RifleActor.generated.h"

UCLASS()
class STALKER_API ARifleActor : public AWeaponActor
{
	GENERATED_BODY()

public:
	ARifleActor();

	virtual void OnBindItem() override;
	virtual void OnUnbindItem(UItemObject* PrevItemObject) override;
	
	virtual void OnFireStart() override;
	virtual void OnFireStop() override;
};
