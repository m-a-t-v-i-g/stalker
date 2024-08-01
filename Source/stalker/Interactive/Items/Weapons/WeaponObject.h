// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactive/Items/ItemObject.h"
#include "WeaponObject.generated.h"

UCLASS()
class STALKER_API UWeaponObject : public UItemObject
{
	GENERATED_BODY()

protected:
	virtual void Use(UObject* Source) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Instance Data")
	FWeaponParams WeaponParams;

public:
	FORCEINLINE FWeaponParams GetWeaponParams() const;
};
