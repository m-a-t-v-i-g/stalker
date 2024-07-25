// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemObject.h"
#include "ItemActor.h"

void UItemObject::InitItem(const uint32 ItemId, const FItemData& ItemData)
{
	ItemParams = ItemData.ItemParams;
	ItemParams.ItemId = ItemId;
	
	ItemDataTable = ItemData.ItemRow.DataTable;
	ItemRowName = ItemData.ItemRow.RowName;
	SetupItemProperties();
}

void UItemObject::SetupItemProperties()
{
	if (!IsStackable())
	{
		ItemParams.Amount = 1;
	}
}

void UItemObject::AddAmount(uint32 Amount)
{
	if (IsStackable())
	{
		ItemParams.Amount += Amount;
	}
}

bool UItemObject::IsSimilar(const UItemObject* OtherItemObject) const
{
	return ItemRowName == OtherItemObject->GetItemRowName();
}

uint32 UItemObject::GetItemId() const
{
	return ItemParams.ItemId;
}

FItemParams UItemObject::GetItemParams() const
{
	return ItemParams;
}

FName UItemObject::GetItemRowName() const
{
	return ItemRowName;
}

FGameplayTag UItemObject::GetItemTag() const
{
	FGameplayTag ItemTag;
	if (GetRow<FTableRowItems>())
	{
		ItemTag = GetRow<FTableRowItems>()->Tag;
	}
	return ItemTag;
}

UClass* UItemObject::GetActorClass() const
{
	UClass* ActorClass = nullptr;
	if (GetRow<FTableRowItems>())
	{
		ActorClass = GetRow<FTableRowItems>()->ActorClass;
	}
	return ActorClass;
}

UClass* UItemObject::GetObjectClass() const
{
	UClass* ObjectClass = nullptr;
	if (GetRow<FTableRowItems>())
	{
		ObjectClass = GetRow<FTableRowItems>()->ObjectClass;
	}
	return ObjectClass;
}

/*
FText UItemObject::GetItemName() const
{
	FText Name;
	if (GetRow<FTableRowItems>())
	{
		Name = GetRow<FTableRowItems>()->Name;
	}
	return Name;
}

FText UItemObject::GetItemDesc() const
{
	FText Description;
	if (GetRow<FTableRowItems>())
	{
		Description = GetRow<FTableRowItems>()->Description;
	}
	return Description;
}
*/

FIntPoint UItemObject::GetItemSize() const
{
	FIntPoint Size = {1, 1};
	if (GetRow<FTableRowItems>())
	{
		Size = GetRow<FTableRowItems>()->Size;
	}
	return Size;
}

bool UItemObject::IsUsable() const
{
	bool bIsUsable = true;
	if (GetRow<FTableRowItems>())
	{
		bIsUsable = GetRow<FTableRowItems>()->bUsable;
	}
	return bIsUsable;
}

bool UItemObject::IsDroppable() const
{
	bool bIsDroppable = true;
	if (GetRow<FTableRowItems>())
	{
		bIsDroppable = GetRow<FTableRowItems>()->bDroppable;
	}
	return bIsDroppable;
}

bool UItemObject::IsStackable() const
{
	bool bIsStackable = true;
	if (GetRow<FTableRowItems>())
	{
		bIsStackable = GetRow<FTableRowItems>()->bStackable;
	}
	return bIsStackable;
}
