// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerComponent.h"
#include "Engine/ActorChannel.h"
#include "InteractiveObjects/Items/ItemObject.h"
#include "Library/Items/ItemsFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

UItemsContainerComponent::UItemsContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UItemsContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemsContainerComponent, Capacity);
	DOREPLIFETIME_CONDITION(UItemsContainerComponent, ItemsContainer,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemsContainerComponent, ItemsSlots,		COND_OwnerOnly);
}

bool UItemsContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
                                                   FReplicationFlags* RepFlags)
{
	bool ReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UItemObject* EachItem : ItemsContainer)
	{
		ReplicateSomething |= Channel->ReplicateSubobject(EachItem, *Bunch, *RepFlags);
	}
	return ReplicateSomething;
}

void UItemsContainerComponent::PreInitializeContainer()
{
	if (!GetOwner()->HasAuthority()) return;
	ItemsSlots.SetNum(Columns * Capacity);
}

void UItemsContainerComponent::PostInitializeContainer()
{
	if (!GetOwner()->HasAuthority()) return;
	AddStartingData();
}

void UItemsContainerComponent::AddStartingData()
{
	for (auto ItemToAdd : StartingData)
	{
		if (!ItemToAdd.IsValid()) continue;

		FItemData ItemData;
		ItemData.ItemRow = ItemToAdd.ItemRow;
		ItemData.ItemParams = ItemToAdd.ItemParams;

		if (auto ItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemData))
		{
			if (!FindAvailablePlace(ItemObject))
			{
				ItemObject->MarkAsGarbage();
			}
		}
	}
}

bool UItemsContainerComponent::FindAvailablePlace(UItemObject* ItemObject)
{
	if (ItemObject)
	{
		if (TryStackItem(ItemObject))
		{
			return true;
		}
		if (TryAddItem(ItemObject))
		{
			return true;
		}
	}
	return false;
}

bool UItemsContainerComponent::TryStackItem(UItemObject* ItemObject)
{
	if (ItemObject)
	{
		if (auto FoundItem = FindAvailableStack(ItemObject))
		{
			StackItem(ItemObject, FoundItem);
			return true;
		}
	}
	return false;
}

bool UItemsContainerComponent::TryAddItem(UItemObject* ItemObject)
{
	if (ItemObject)
	{
		uint32 FoundIndex = FindAvailableRoom(ItemObject);
		if (CanAddItem(ItemObject))
		{
			AddItemAt(ItemObject, FoundIndex);
			return true;
		}
	}
	return false;
}

void UItemsContainerComponent::StackItem(UItemObject* SourceObject, UItemObject* TargetItem)
{
	check(SourceObject);

	if (TargetItem->IsSimilar(SourceObject) && TargetItem->IsStackable())
	{
		TargetItem->AddAmount(SourceObject->GetItemParams().Amount);
		
		ClearSlotsByItemId(SourceObject->GetItemId());
	
		if (ItemsContainer.Contains(SourceObject))
		{
			ItemsContainer.Remove(SourceObject);
		}
		
		if (GetOwner()->HasAuthority())
		{
			SourceObject->MarkAsGarbage();
		}
	}

	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_StackItem(SourceObject, TargetItem);
	}
}

void UItemsContainerComponent::Server_StackItem_Implementation(UItemObject* SourceObject, UItemObject* TargetItem)
{
	StackItem(SourceObject, TargetItem);
}

void UItemsContainerComponent::StackItemAt(UItemObject* ItemObject, uint32 Index)
{
	check(ItemObject);

	uint32 ItemId = ItemsSlots[Index];
	if (auto FoundItem = FindItemById(ItemId))
	{
		if (FoundItem->IsSimilar(ItemObject) && FoundItem->IsStackable())
		{
			FoundItem->AddAmount(ItemObject->GetItemParams().Amount);
			
			if (ItemsContainer.Contains(ItemObject))
			{
				ItemsContainer.Remove(ItemObject);
			}
		}
	}

	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_StackItemAt(ItemObject, Index);
	}
}

void UItemsContainerComponent::Server_StackItemAt_Implementation(UItemObject* ItemObject, uint32 Index)
{
	StackItemAt(ItemObject, Index);
}

void UItemsContainerComponent::SplitItem(UItemObject* ItemObject)
{
	check(ItemObject);

	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_SplitItem(ItemObject);
	}
}

void UItemsContainerComponent::Server_SplitItem_Implementation(UItemObject* ItemObject)
{
	ItemObject->RemoveAmount(ItemObject->GetItemParams().Amount / 2);
	
	if (auto NewItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemObject))
	{
		TryAddItem(NewItemObject);
		
		if (!ItemsContainer.Contains(NewItemObject))
		{
			ItemsContainer.Add(NewItemObject);
		}
	}
}

void UItemsContainerComponent::AddItemAt(UItemObject* ItemObject, uint32 Index)
{
	check(ItemObject);
	ItemObject->SetInventoriedMode();
	
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};
	
	FillRoom(ItemsSlots, ItemObject->GetItemId(), Tile, ItemSize, Columns);
	
	if (!ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Add(ItemObject);
	}
	
	UpdateItemsMap();
	
	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_AddItemAt(ItemObject, Index);
	}
}

void UItemsContainerComponent::Server_AddItemAt_Implementation(UItemObject* ItemObject, uint32 Index)
{
	AddItemAt(ItemObject, Index);
}

void UItemsContainerComponent::RemoveItem(UItemObject* ItemObject, bool bUpdateItemsMap)
{
	check(ItemObject);
	
	ClearSlotsByItemId(ItemObject->GetItemId());
	
	if (ItemsContainer.Contains(ItemObject))
	{
		ItemsContainer.Remove(ItemObject);
	}

	if (bUpdateItemsMap)
	{
		UpdateItemsMap();
	}
	
	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_RemoveItem(ItemObject, bUpdateItemsMap);
	}
}

void UItemsContainerComponent::Server_RemoveItem_Implementation(UItemObject* ItemObject, bool bUpdateItemsMap)
{
	RemoveItem(ItemObject, bUpdateItemsMap);
}

void UItemsContainerComponent::MoveItemToOtherContainer(UItemObject* ItemObject,
                                                        UItemsContainerComponent* OtherContainer)
{
	check(ItemObject && OtherContainer);
	
	Server_MoveItemToOtherContainer(ItemObject, OtherContainer);
}

void UItemsContainerComponent::Server_MoveItemToOtherContainer_Implementation(
	UItemObject* ItemObject, UItemsContainerComponent* OtherContainer)
{
	if (ItemObject->GetItemParams().Amount > ItemObject->GetStackAmount())
	{
		ItemObject->RemoveAmount(ItemObject->GetStackAmount());
		
		if (auto NewItemObject = UItemsFunctionLibrary::GenerateItemObject(ItemObject))
		{
			NewItemObject->SetAmount(NewItemObject->GetStackAmount());
			OtherContainer->FindAvailablePlace(NewItemObject);
		}
	}
	else
	{
		OtherContainer->FindAvailablePlace(ItemObject);
		RemoveItem(ItemObject);
	}
}

void UItemsContainerComponent::SubtractOrRemoveItem(UItemObject* ItemObject)
{
	check(ItemObject);

	if (ItemObject->GetItemParams().Amount > 1)
	{
		ItemObject->RemoveAmount(1);
	}
	else
	{
		if (GetOwner()->HasAuthority())
		{
			RemoveItem(ItemObject, false);
			ItemObject->MarkAsGarbage();
		}
	}
	
	UpdateItemsMap();
	
	if (!GetOwner()->HasAuthority() && GetOwnerRole() != ROLE_SimulatedProxy)
	{
		Server_SubtractOrRemoveItem(ItemObject);
	}
}

void UItemsContainerComponent::Server_SubtractOrRemoveItem_Implementation(UItemObject* ItemObject)
{
	SubtractOrRemoveItem(ItemObject);
}

void UItemsContainerComponent::ClearSlotsByItemId(uint32 ItemId)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (ItemsSlots[i] == ItemId)
		{
			ItemsSlots[i] = 0;
		}
	}
}

void UItemsContainerComponent::UpdateItemsMap()
{
	ItemsMap.Empty();
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (ItemsSlots[i] != 0)
		{
			if (!ItemsMap.Contains(ItemsSlots[i]))
			{
				ItemsMap.Add(ItemsSlots[i], TileFromIndex(i, Columns));
			}
		}
	}
	OnItemsContainerUpdated.Broadcast();
}

bool UItemsContainerComponent::CanStackAtRoom(const UItemObject* ItemObject, uint32 RoomIndex)
{
	bool bResult = false;
	if (ItemObject)
	{
		if (auto FoundItem = ItemsContainer[RoomIndex])
		{
			bResult = FoundItem != ItemObject && FoundItem->IsSimilar(ItemObject) && FoundItem->IsStackable();
		}
	}
	return bResult;
}

bool UItemsContainerComponent::CanAddItem(const UItemObject* ItemObject) const
{
	if (!ItemObject)
	{
		return false;
	}
	
	bool bResult = ItemObject->GetItemTag().MatchesAny(CategoryTags);
	bResult &= Columns >= ItemObject->GetItemSize().X;
	return bResult;
}

UItemObject* UItemsContainerComponent::FindAvailableStack(const UItemObject* ItemObject)
{
	for (int i = 0; i < ItemsContainer.Num(); i++)
	{
		if (CanStackAtRoom(ItemObject, i))
		{
			return ItemsContainer[i];
		}
	}
	return nullptr;
}

uint32 UItemsContainerComponent::FindAvailableRoom(const UItemObject* ItemObject)
{
	for (int i = 0; i < ItemsSlots.Num(); i++)
	{
		if (CheckRoom(ItemObject, i))
		{
			return i;
		}
	}
	return 0;
}

bool UItemsContainerComponent::CheckRoom(const UItemObject* ItemObject, uint32 Index)
{
	const FIntPoint Tile = TileFromIndex(Index, Columns);
	const FIntPoint ItemSize = {Tile.X + (ItemObject->GetItemSize().X - 1), Tile.Y + (ItemObject->GetItemSize().Y - 1)};

	return IsRoomValid(ItemsSlots, Tile, ItemSize, Columns);
}

UItemObject* UItemsContainerComponent::FindItemById(uint32 ItemId) const
{
	UItemObject* FoundItem = nullptr;
	for (auto EachItem : ItemsContainer)
	{
		if (EachItem->GetItemId() == ItemId)
		{
			FoundItem = EachItem;
		}
	}
	return FoundItem;
}

void UItemsContainerComponent::OnRep_ItemsSlots()
{
	UpdateItemsMap();
}

FIntPoint UItemsContainerComponent::TileFromIndex(uint32 Index, uint8 Width)
{
	return {(int)Index % Width, (int)Index / Width};
}

uint32 UItemsContainerComponent::IndexFromTile(const FIntPoint& Tile, int Width)
{
	return Tile.X + Tile.Y * Width;
}

void UItemsContainerComponent::FillRoom(TArray<uint32>& Slots, uint32 ItemId, const FIntPoint& Tile,
                                        const FIntPoint& ItemSize, uint8 Columns)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			Slots[IndexFromTile({x, y}, Columns)] = ItemId;
		}
	}
}

bool UItemsContainerComponent::IsRoomValid(TArray<uint32>& Slots, const FIntPoint& Tile, const FIntPoint& ItemSize,
                                           uint8 Columns)
{
	for (int x = Tile.X; x <= ItemSize.X; x++)
	{
		for (int y = Tile.Y; y <= ItemSize.Y; y++)
		{
			if (IsItemSizeValid({x, y}, Columns))
			{
				if (IsTileFilled(Slots, IndexFromTile({x, y}, Columns)))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}

bool UItemsContainerComponent::IsItemSizeValid(const FIntPoint& ItemSize, uint8 Columns)
{
	return ItemSize.X >= 0 && ItemSize.Y >= 0 && ItemSize.X < Columns;
}

bool UItemsContainerComponent::IsTileFilled(const TArray<uint32>& Slots, uint32 Index)
{
	return Slots[Index] != 0;
}
