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

void UItemInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	ItemId = NewItemId;

	if (Definition)
	{
		ItemDefinition = Definition;
		
		if (Instance)
		{
			Amount = Instance->Amount;
			Endurance = Instance->Endurance;
		}
	}
}

void UItemObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UItemObject, BoundItem,		COND_OwnerOnly);
}

void UItemObject::Use_Implementation(UObject* Source)
{
	IUsableInterface::Use_Implementation(Source);
}

void UItemObject::InitItem(const uint32 ItemId, const UItemObject* ItemObject)
{
	ItemDefinition = ItemObject->ItemDefinition;

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
	ItemInstance->SetupProperties(ItemId, ItemDefinition, ItemObject->ItemInstance);
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

void UItemObject::SetGrounded()
{
	ItemInstance->Mode = EItemMode::Grounded;
}

void UItemObject::SetCollected()
{
	ItemInstance->Mode = EItemMode::Collected;
}

void UItemObject::SetEquipped()
{
	ItemInstance->Mode = EItemMode::Equipped;
}

void UItemObject::SetAmount(uint32 Amount) const
{
	if (ItemInstance)
	{
		ItemInstance->Amount = Amount;
	}
}

void UItemObject::AddAmount(uint32 Amount) const
{
	if (ItemInstance)
	{
		ItemInstance->Amount += Amount;
	}
}

void UItemObject::RemoveAmount(uint32 Amount) const
{
	if (ItemInstance)
	{
		ItemInstance->Amount -= Amount;
	}
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
	bool bResult = ItemDefinition == OtherItemObject->ItemDefinition;
	if (bResult)
	{
		
	}
	return bResult;
}

uint32 UItemObject::GetItemId() const
{
	return ItemInstance->ItemId;
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
	return ItemDefinition->bStackable;
}

uint32 UItemObject::GetStackAmount() const
{
	return ItemDefinition->StackAmount;
}

bool UItemObject::CanStackItem(const UItemObject* OtherItem) const
{
	return this != OtherItem && IsStackable() && IsSimilar(OtherItem);
}
