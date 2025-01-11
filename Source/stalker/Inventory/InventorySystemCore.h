// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ItemsContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventorySystemCore.generated.h"

class UItemsContainer;
class UEquipmentSlot;
class UItemObject;

UCLASS()
class STALKER_API UInventorySystemCore : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool FindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject);
	static bool StackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem);
	static void AddItem(UItemsContainer* Container, UItemObject* ItemObject);
	static void SplitItem(UItemsContainer* Container, UItemObject* ItemObject);
	static void RemoveItem(UItemsContainer* Container, UItemObject* ItemObject);
	static bool SubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount);
	static void MoveItemToOtherContainer(UItemsContainer* SourceContainer, UItemsContainer* TargetContainer,
	                                     UItemObject* ItemObject);

	static void TryEquipItem(const TArray<UEquipmentSlot*>& Slots, UItemObject* ItemObject,
	                         UItemsContainer* Container = nullptr);
	static void EquipSlot(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject);
	static void UnequipSlot(UEquipmentSlot* EquipmentSlot);
	static void MoveItemFromEquipmentSlot(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container);
};
