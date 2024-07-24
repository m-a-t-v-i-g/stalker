// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "EquipmentSlot.generated.h"

UCLASS()
class STALKER_API UEquipmentSlot : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Equipment Slot")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditInstanceOnly, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditInstanceOnly, Category = "Equipment Slot")
	FGameplayTag ItemTag;

	UPROPERTY(EditInstanceOnly, Category = "Equipment Slot")
	TWeakObjectPtr<const UObject> BoundObject;

	UPROPERTY(EditInstanceOnly, Category = "Equipment Slot")
	bool bAvailable = true;

public:
	const FString& GetSlotName() const { return SlotName; }
};
