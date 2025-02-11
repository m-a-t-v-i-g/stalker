// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemSystemCore.h"
#include "AbilitySet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ItemObject.h"
#include "OutfitComponent.h"
#include "Game/StalkerGameState.h"
#include "Kismet/GameplayStatics.h"

uint32 UItemSystemCore::LastItemId {0};

namespace FItemSystemTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemTag_Item,				"Item", "Default item tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemTag_Armor,				"Item.Armor", "Default armor tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemTag_ArmorBody,			"Item.Armor.Body", "Armor body tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemTag_Weapon,				"Item.Weapon", "Default weapon tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemTag_WeaponPrimary,		"Item.Weapon.Primary", "Weapon primary tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemTag_WeaponSecondary,		"Item.Weapon.Secondary", "Weapon secondary tag.");
}

void FOutfitList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	
}

void FOutfitList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

UItemObject* FOutfitList::AddEntry(const FString& SlotName, UItemObject* ItemObject)
{
	check(ItemObject);
	
	FAppliedOutfitEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.SlotName = SlotName;
	NewEntry.ItemObject = ItemObject;

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
	{
		if (const UAbilitySet* AbilitySet = ItemObject->GetAbilitySet())
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, NewEntry.Abilities, ItemObject);
		}
	}

	// TODO: Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);
	
	MarkItemDirty(NewEntry);
	return NewEntry.ItemObject;
}

void FOutfitList::RemoveEntry(const FString& SlotName)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FAppliedOutfitEntry& Entry = *EntryIt;
		if (Entry.SlotName == SlotName)
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
			{
				for (FGameplayAbilitySpecHandle& EachAbility : Entry.Abilities)
				{
					AbilitySystemComponent->ClearAbility(EachAbility);
				}
			}

			// TODO: Instance->DestroyEquipmentActors();
			
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
			break;
		}
	}
}

UAbilitySystemComponent* FOutfitList::GetAbilitySystemComponent() const
{
	check(OutfitComponent);
	
	AActor* OwningActor = OutfitComponent->GetOwner();
	check(OwningActor);
	
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor);
}

UItemObject* UItemSystemCore::GenerateItemObject(UWorld* World, const UItemObject* ItemObject)
{
	if (ItemObject)
	{
		if (UItemObject* NewItemObject = NewObject<UItemObject>(World, ItemObject->GetObjectClass(),
		                                                        FName(ItemObject->GetScriptName().ToString() +
			                                                        FString::Printf(
				                                                        TEXT("_object%d"), LastItemId + 1))))
		{
			LastItemId++;
			AddItemObjectToGameState(NewItemObject);
			NewItemObject->InitItem(LastItemId, ItemObject);
			return NewItemObject;
		}
	}
	return nullptr;
}

UItemObject* UItemSystemCore::GenerateItemObject(UWorld* World, const UItemDefinition* Definition,
                                                 const UItemPredictedData* PredictedData)
{
	if (Definition)
	{
		if (UItemObject* ItemObject = NewObject<UItemObject>(World, Definition->ItemObjectClass,
		                                                     FName(Definition->ScriptName.ToString() + FString::Printf(
			                                                     TEXT("_object%d"), LastItemId + 1))))
		{
			LastItemId++;
			AddItemObjectToGameState(ItemObject);
			ItemObject->InitItem(LastItemId, Definition, PredictedData);
			return ItemObject;
		}
	}
	return nullptr;
}

void UItemSystemCore::DestroyItemObject(const UItemObject* ItemObject)
{
	if (ItemObject)
	{
		RemoveItemObjectFromGameState(ItemObject);
	}
}

UItemObject* UItemSystemCore::GetItemObjectById(const UWorld* World, uint32 ItemId)
{
	if (ItemId > 0)
	{
		if (auto GameState = Cast<AStalkerGameState>(World->GetGameState()))
		{
			return GameState->GetItemObjectById(ItemId);
		}
	}
	return nullptr;
}

bool UItemSystemCore::IsItemObjectValid(const UWorld* World, uint32 ItemId)
{
	if (ItemId > 0)
	{
		if (auto GameState = Cast<AStalkerGameState>(World->GetGameState()))
		{
			return GameState->IsItemObjectExist(ItemId);
		}
	}
	return false;
}

void UItemSystemCore::AddItemObjectToGameState(UItemObject* ItemObject)
{
	if (auto GameState = Cast<AStalkerGameState>(UGameplayStatics::GetGameState(ItemObject)))
	{
		GameState->AddItemObject(LastItemId, ItemObject);
	}
}

void UItemSystemCore::RemoveItemObjectFromGameState(const UItemObject* ItemObject)
{
	if (auto GameState = Cast<AStalkerGameState>(UGameplayStatics::GetGameState(ItemObject)))
	{
		GameState->RemoveItemObject(ItemObject->GetItemId());
	}
}

