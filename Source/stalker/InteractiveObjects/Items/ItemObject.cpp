// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemObject.h"
#include "ItemActor.h"
#include "Net/UnrealNetwork.h"

void UItemObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UItemObject, ItemParams,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemObject, ItemDataTable, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemObject, ItemRowName,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemObject, bStackable,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemObject, BoundItem,		COND_OwnerOnly);
}

void UItemObject::Use_Implementation(UObject* Source)
{
	IUsableInterface::Use_Implementation(Source);
}

void UItemObject::InitItem(const uint32 ItemId, const FDataTableRowHandle& RowHandle)
{
	ItemParams.ItemId = ItemId;
	
	ItemDataTable = RowHandle.DataTable;
	ItemRowName = RowHandle.RowName;
	SetupItemProperties();
}

void UItemObject::InitItem(const uint32 ItemId, const UItemObject* ItemObject)
{
	ItemParams = ItemObject->GetItemParams();
	ItemParams.ItemId = ItemId;

	ItemDataTable = ItemObject->GetItemDataTable();
	ItemRowName = ItemObject->GetItemRowName();
	SetupItemProperties();
}

void UItemObject::SetupItemProperties()
{
	bStackable = GetRow<FTableRowItems>()->bStackable;
	
	if (!IsStackable())
	{
		ItemParams.Amount = 1;
	}
}

void UItemObject::BindItem(AItemActor* BindItem)
{
	if (!IsValid(BindItem)) return;

	BoundItem = BindItem;
	OnBindItem();
}

void UItemObject::OnBindItem()
{
}

void UItemObject::UnbindItem()
{
	if (!BoundItem.Get()) return;

	BoundItem = nullptr;
	OnUnbindItem();
}

void UItemObject::OnUnbindItem()
{
}

void UItemObject::SetInventoriedMode()
{
	if (GetRow<FTableRowItems>())
	{
		bStackable = GetRow<FTableRowItems>()->bStackable;
	}
}

void UItemObject::SetEquippedMode()
{
	SetAmount(1);
	bStackable = false;
}

void UItemObject::SetAmount(uint32 Amount)
{
	ItemParams.Amount = Amount;
}

void UItemObject::AddAmount(uint32 Amount)
{
	ItemParams.Amount += Amount;
}

void UItemObject::RemoveAmount(uint32 Amount)
{
	ItemParams.Amount -= Amount;
}

void UItemObject::OnRep_BoundItem()
{
	if (BoundItem)
	{
		OnBindItem();
	}
}

bool UItemObject::IsSimilar(const UItemObject* OtherItemObject) const
{
	bool bResult = ItemRowName == OtherItemObject->GetItemRowName();
	if (bResult)
	{
		bResult &= ItemParams == OtherItemObject->GetItemParams();
	}
	return bResult;
}

uint32 UItemObject::GetItemId() const
{
	return ItemParams.ItemId;
}

FItemParams UItemObject::GetItemParams() const
{
	return ItemParams;
}

const UDataTable* UItemObject::GetItemDataTable() const
{
	return ItemDataTable;
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

UTexture2D* UItemObject::GetThumbnail() const
{
	UTexture2D* Thumbnail = nullptr;
	if (GetRow<FTableRowItems>())
	{
		Thumbnail = GetRow<FTableRowItems>()->Thumbnail.LoadSynchronous();
	}
	return Thumbnail;
}

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
	return bStackable & bIsStackable;
}

uint32 UItemObject::GetStackAmount() const
{
	uint32 StackAmount = 1;
	if (GetRow<FTableRowItems>())
	{
		StackAmount = GetRow<FTableRowItems>()->StackAmount;
	}
	return StackAmount;
}

UStaticMesh* UItemObject::GetPreviewMesh() const
{
	UStaticMesh* StaticMesh = nullptr;
	if (GetRow<FTableRowItems>())
	{
		StaticMesh = GetRow<FTableRowItems>()->PreviewMesh.LoadSynchronous();
	}
	return StaticMesh;
}
