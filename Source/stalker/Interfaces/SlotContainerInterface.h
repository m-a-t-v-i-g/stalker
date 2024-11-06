// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SlotContainerInterface.generated.h"

class UEquipmentSlot;
class UItemObject;

UINTERFACE()
class USlotContainerInterface : public UInterface
{
	GENERATED_BODY()
};

class STALKER_API ISlotContainerInterface
{
	GENERATED_BODY()

public:
	virtual void EquipSlot(const FString& SlotName, UItemObject* ItemObject);
	
	virtual void UnequipSlot(const FString& SlotName);

	virtual bool CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject);

	virtual UEquipmentSlot* FindEquipmentSlot(const FString& SlotName) const;
};
