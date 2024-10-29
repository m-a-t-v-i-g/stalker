// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsFunctionLibrary.h"
#include "InteractiveObjects/ItemSystem/ItemObject.h"

uint32 UItemsFunctionLibrary::LastItemId {0};

UItemObject* UItemsFunctionLibrary::GenerateItemObject(const FDataTableRowHandle& RowHandle)
{
	UItemObject* ItemObject = nullptr;
	if (auto DataTable = RowHandle.DataTable)
	{
		auto OutRow = DataTable->FindRow<FTableRowItems>(RowHandle.RowName, "");
		if (UClass* ItemClass = OutRow->ObjectClass)
		{
			ItemObject = NewObject<UItemObject>(GetTransientPackage(), ItemClass);
			if (ItemObject)
			{
				LastItemId++;
				ItemObject->InitItem(LastItemId, RowHandle);
			}
		}
	}
	return ItemObject;
}

UItemObject* UItemsFunctionLibrary::GenerateItemObject(const UItemObject* ItemObject)
{
	UItemObject* NewItemObject = NewObject<UItemObject>(GetTransientPackage(), ItemObject->GetObjectClass());
	if (NewItemObject)
	{
		LastItemId++;
		NewItemObject->InitItem(LastItemId, ItemObject);
	}
	return NewItemObject;
}

UItemObject* UItemsFunctionLibrary::GenerateItemObject(const UItemDefinition* Definition, const UItemPredictedData* PredictedData)
{
	UItemObject* ItemObject = nullptr;
	if (Definition)
	{
		ItemObject = NewObject<UItemObject>(GetTransientPackage(), Definition->ObjectClass);
		if (ItemObject)
		{
			LastItemId++;
			ItemObject->InitItem(LastItemId, Definition, PredictedData);
		}
	}
	return ItemObject;
}
