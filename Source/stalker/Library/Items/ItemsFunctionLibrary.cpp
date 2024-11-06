// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsFunctionLibrary.h"
#include "ItemObject.h"
#include "Game/StalkerGameState.h"
#include "Kismet/GameplayStatics.h"

uint32 UItemsFunctionLibrary::LastItemId {0};

UItemObject* UItemsFunctionLibrary::GenerateItemObject(UWorld* World, UItemObject* ItemObject)
{
	UItemObject* NewItemObject = NewObject<UItemObject>(World, ItemObject->GetObjectClass());
	if (NewItemObject)
	{
		LastItemId++;
		AddItemObjectToGameState(NewItemObject);
		NewItemObject->InitItem(LastItemId, ItemObject);
	}
	return NewItemObject;
}

UItemObject* UItemsFunctionLibrary::GenerateItemObject(UWorld* World, const UItemDefinition* Definition,
                                                       const UItemPredictedData* PredictedData)
{
	UItemObject* ItemObject = nullptr;
	if (Definition)
	{
		ItemObject = NewObject<UItemObject>(World, Definition->ObjectClass);
		if (ItemObject)
		{
			LastItemId++;
			AddItemObjectToGameState(ItemObject);
			ItemObject->InitItem(LastItemId, Definition, PredictedData);
		}
	}
	return ItemObject;
}

void UItemsFunctionLibrary::DestroyItemObject(const UItemObject* ItemObject)
{
	if (ItemObject)
	{
		RemoveItemObjectFromGameState(ItemObject);
	}
}

UItemObject* UItemsFunctionLibrary::GetItemObjectById(const UWorld* World, uint32 ItemId)
{
	if (auto GameState = Cast<AStalkerGameState>(World->GetGameState()))
	{
		return GameState->GetItemObjectById(ItemId);
	}
	return nullptr;
}

bool UItemsFunctionLibrary::IsItemObjectExist(const UWorld* World, uint32 ItemId)
{
	if (auto GameState = Cast<AStalkerGameState>(World->GetGameState()))
	{
		return GameState->IsItemObjectExist(ItemId);
	}
	return false;
}

void UItemsFunctionLibrary::AddItemObjectToGameState(UItemObject* ItemObject)
{
	if (auto GameState = Cast<AStalkerGameState>(UGameplayStatics::GetGameState(ItemObject)))
	{
		GameState->AddItemObject(LastItemId, ItemObject);
	}
}

void UItemsFunctionLibrary::RemoveItemObjectFromGameState(const UItemObject* ItemObject)
{
	if (auto GameState = Cast<AStalkerGameState>(UGameplayStatics::GetGameState(ItemObject)))
	{
		GameState->RemoveItemObject(ItemObject->GetItemId());
	}
}

