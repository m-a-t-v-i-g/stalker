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
	static bool Container_FindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject);
	static bool Container_StackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem);
	static void Container_SplitItem(UItemsContainer* Container, UItemObject* ItemObject);
	static void Container_AddItem(UItemsContainer* Container, UItemObject* ItemObject);
	static void Container_RemoveItem(UItemsContainer* Container, UItemObject* ItemObject);
	static bool Container_SubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount);
	static void Container_MoveItemToOtherContainer(UItemsContainer* SourceContainer, UItemsContainer* TargetContainer,
	                                               UItemObject* ItemObject, bool bFullStack = false);

	static void Slot_EquipItem(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject);
	static void Slot_UnequipItem(UEquipmentSlot* EquipmentSlot);
	static void Slot_MoveItemToContainer(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container);
};
