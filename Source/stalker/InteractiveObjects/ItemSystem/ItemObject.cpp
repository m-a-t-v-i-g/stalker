// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemObject.h"
#include "ItemActor.h"
#include "Net/UnrealNetwork.h"

void UItemInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                    const UItemPredictedData* PredictedData)
{
	ItemId = NewItemId;

	if (Definition)
	{
		ItemDefinition = Definition;
		
		if (PredictedData)
		{
			Amount = PredictedData->Amount;
			Endurance = PredictedData->Endurance;
		}
	}
}

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

void UItemObject::InitItem(const uint32 ItemId, const UItemDefinition* Definition, const UItemPredictedData* PredictedData)
{
	ItemDefinition = Definition;
	
	if (!ItemDefinition)
	{
		return;
	}

	UItemInstance* NewItemInstance = NewObject<UItemInstance>(this, ItemDefinition->ItemInstanceClass);
	if (!NewItemInstance)
	{
		return;
	}

	ItemInstance = NewItemInstance;
	ItemInstance->SetupProperties(ItemId, ItemDefinition, PredictedData);
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
	return ItemInstance->ItemId;
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
	return ItemDefinition->Tag;
}

UClass* UItemObject::GetActorClass() const
{
	return ItemDefinition->ActorClass;
}

UClass* UItemObject::GetObjectClass() const
{
	return ItemDefinition->ObjectClass;
}

FText UItemObject::GetItemName() const
{
	return ItemDefinition->Name;
}

FText UItemObject::GetItemDesc() const
{
	return ItemDefinition->Description;
}

UTexture2D* UItemObject::GetThumbnail() const
{
	return ItemDefinition->Thumbnail.LoadSynchronous();
}

FIntPoint UItemObject::GetItemSize() const
{
	return ItemDefinition->Size;
}

bool UItemObject::IsUsable() const
{
	return ItemDefinition->bUsable;
}

bool UItemObject::IsDroppable() const
{
	return ItemDefinition->bDroppable;
}

bool UItemObject::IsStackable() const
{
	return bStackable & ItemDefinition->bStackable;
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
