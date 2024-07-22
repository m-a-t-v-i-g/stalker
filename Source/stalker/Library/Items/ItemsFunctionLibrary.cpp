// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsFunctionLibrary.h"
#include "Interactive/Items/ItemObject.h"

uint32 UItemsFunctionLibrary::LastItemId {0};

UItemObject* UItemsFunctionLibrary::GenerateItemObject(const FItemData& ItemData)
{
	UItemObject* ItemObject = nullptr;
	if (auto DataTable = ItemData.ItemRow.DataTable)
	{
		auto OutRow = DataTable->FindRow<FTableRowItems>(ItemData.ItemRow.RowName, "");
		if (UClass* ItemClass = OutRow->ObjectClass)
		{
			ItemObject = NewObject<UItemObject>(GetTransientPackage(), ItemClass);
			if (ItemObject)
			{
				LastItemId++;
				ItemObject->InitItem(LastItemId, ItemData);
			}
		}
	}
	return ItemObject;
}
