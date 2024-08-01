// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Inventory/ItemsContainerComponent.h"
#include "InventoryComponent.generated.h"

class UItemObject;

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UInventoryComponent : public UItemsContainerComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	void DropItem(UItemObject* ItemObject);
	
	void UseItem(UItemObject* ItemObject);

	UFUNCTION(Server, Reliable)
	void Server_UseItem(UItemObject* ItemObject);
};
