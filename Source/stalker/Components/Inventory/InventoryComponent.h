// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Items/ItemsContainerComponent.h"
#include "InventoryComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UInventoryComponent : public UItemsContainerComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	void DropItem(UItemObject* ItemObject);
};
