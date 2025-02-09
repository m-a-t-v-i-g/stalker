// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySystemCore.h"
#include "EquipmentSlot.h"
#include "ItemSystemCore.h"

bool UInventorySystemCore::Container_FindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return false;
	}

	return Container->FindAvailablePlace(ItemObject);
}

bool UInventorySystemCore::Container_StackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem)
{
	if (!Container || !SourceItem || !TargetItem)
	{
		return false;
	}

	return Container->StackItem(SourceItem, TargetItem);
}

void UInventorySystemCore::Container_SplitItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return;
	}

	Container->SplitItem(ItemObject);
}

void UInventorySystemCore::Container_AddItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return;
	}

	Container->AddItem(ItemObject);
}

void UInventorySystemCore::Container_RemoveItem(UItemsContainer* Container, UItemObject* ItemObject)
{
	if (!Container || !ItemObject)
	{
		return;
	}

	Container->RemoveItem(ItemObject);
}

bool UInventorySystemCore::Container_SubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount)
{
	if (!Container || !ItemObject || Amount <= 0)
	{
		return false;
	}

	return Container->SubtractOrRemoveItem(ItemObject, Amount);
}

void UInventorySystemCore::Container_MoveItemToOtherContainer(UItemsContainer* SourceContainer, UItemsContainer* TargetContainer,
                                                    UItemObject* ItemObject, bool bFullStack)
{
	if (!SourceContainer || !TargetContainer || !ItemObject)
	{
		return;
	}
	
	if (!bFullStack && ItemObject->GetAmount() > ItemObject->GetStackAmount())
	{
		if (UItemObject* StackableItem = TargetContainer->FindAvailableStack(ItemObject))
		{
			StackableItem->AddAmount(ItemObject->GetStackAmount());
		}
		else if (UItemObject* NewItemObject = UItemSystemCore::GenerateItemObject(TargetContainer->GetWorld(), ItemObject))
		{
			NewItemObject->SetAmount(NewItemObject->GetStackAmount());
			Container_AddItem(TargetContainer, NewItemObject);
		}
		
		ItemObject->RemoveAmount(ItemObject->GetStackAmount());
	}
	else
	{
		if (Container_FindAvailablePlace(TargetContainer, ItemObject))
		{
			Container_RemoveItem(SourceContainer, ItemObject);
		}
	}
}

void UInventorySystemCore::Slot_EquipItem(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject)
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

void UInventorySystemCore::Slot_UnequipItem(UEquipmentSlot* EquipmentSlot)
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

void UInventorySystemCore::Slot_MoveItemToContainer(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container)
{
	if (!EquipmentSlot || !Container)
	{
		return;
	}

	if (UItemObject* OldBoundObject = EquipmentSlot->GetBoundObject())
	{
		Slot_UnequipItem(EquipmentSlot);
		Container_AddItem(Container, OldBoundObject);
	}
}
