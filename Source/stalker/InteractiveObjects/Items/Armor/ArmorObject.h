// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveObjects/Items/ItemObject.h"
#include "ArmorObject.generated.h"

UCLASS()
class STALKER_API UArmorObject : public UItemObject
{
	GENERATED_BODY()

protected:
	virtual void Use_Implementation(UObject* Source) override;

public:
	FORCEINLINE USkeletalMesh* GetVisual() const;
};
