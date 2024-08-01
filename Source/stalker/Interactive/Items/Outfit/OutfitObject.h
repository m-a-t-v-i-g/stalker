// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactive/Items/ItemObject.h"
#include "OutfitObject.generated.h"

UCLASS()
class STALKER_API UOutfitObject : public UItemObject
{
	GENERATED_BODY()

protected:
	virtual void Use(UObject* Source) override;

public:
	FORCEINLINE USkeletalMesh* GetDefaultMesh() const;
	FORCEINLINE USkeletalMesh* GetHelmetMesh() const;
};
