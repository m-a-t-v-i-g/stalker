// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySystemCore.h"
#include "EquipmentSlot.h"
#include "ItemSystemCore.h"

bool UInventorySystemCore::FindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return false;
	}

	return Container->FindAvailablePlace(ItemObject);
}

bool UInventorySystemCore::StackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (!Container || !SourceItem || !TargetItem)
	{
		return false;
	}

	return Container->StackItem(SourceItem, TargetItem);
}

void UInventorySystemCore::AddItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return;
	}

	Container->AddItem(ItemObject);
}

void UInventorySystemCore::SplitItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return;
	}

	Container->SplitItem(ItemObject);
}

void UInventorySystemCore::RemoveItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return;
	}

	Container->RemoveItem(ItemObject);
}

bool UInventorySystemCore::SubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount)
{
	if (!Container || !ItemObject || Amount <= 0)
	{
		return false;
	}

	return Container->SubtractOrRemoveItem(ItemObject, Amount);
}

void UInventorySystemCore::MoveItemToOtherContainer(UItemsContainer* SourceContainer, UItemsContainer* TargetContainer,
                                                    UItemObject* ItemObject)
{
	if (!SourceContainer || !TargetContainer || !ItemObject)
	{
		return;
	}
	
	if (ItemObject->GetAmount() > ItemObject->GetStackAmount())
	{
		if (UItemObject* StackableItem = TargetContainer->FindAvailableStack(ItemObject))
		{
			StackableItem->AddAmount(ItemObject->GetStackAmount());
		}
		else if (UItemObject* NewItemObject = UItemSystemCore::GenerateItemObject(TargetContainer->GetWorld(), ItemObject))
		{
			NewItemObject->SetAmount(NewItemObject->GetStackAmount());
			TargetContainer->AddItem(NewItemObject);
		}
		ItemObject->RemoveAmount(ItemObject->GetStackAmount());
	}
	else
	{
		if (TargetContainer->FindAvailablePlace(ItemObject))
		{
			SourceContainer->RemoveItem(ItemObject);
		}
	}
}

void UInventorySystemCore::TryEquipItem(const TArray<UEquipmentSlot*>& Slots, UItemObject* ItemObject,
                                        UItemsContainer* Container)
{
}

void UInventorySystemCore::EquipSlot(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject)
{
	if (!EquipmentSlot || !ItemObject)
	{
		return;
	}

	if (!EquipmentSlot->IsEquipped())
	{
		EquipmentSlot->EquipSlot(ItemObject);
	}
}

void UInventorySystemCore::UnequipSlot(UEquipmentSlot* EquipmentSlot)
{
	if (!EquipmentSlot)
	{
		return;
	}

	if (EquipmentSlot->IsEquipped())
	{
		EquipmentSlot->UnequipSlot();
	}
}

void UInventorySystemCore::MoveItemFromEquipmentSlot(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container)
{
	if (!EquipmentSlot || !Container)
	{
		return;
	}

	if (UItemObject* OldBoundObject = EquipmentSlot->GetBoundObject())
	{
		UnequipSlot(EquipmentSlot);
		AddItem(Container, OldBoundObject);
	}
}
