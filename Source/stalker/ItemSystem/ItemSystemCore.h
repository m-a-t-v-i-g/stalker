// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ItemSystemCore.generated.h"

struct FAppliedOutfitEntry;
struct FOutfitList;
struct FGameplayAbilitySpecHandle;

class UGameplayEffect;
class UItemDefinition;
class UItemPredictedData;
class UItemObject;
class AItemActor;

USTRUCT()
struct FAppliedOutfitEntry : public FFastArraySerializerItem
{
	GENERATED_USTRUCT_BODY()

	FAppliedOutfitEntry()
	{
	}

private:
	friend FOutfitList;

	UPROPERTY()
	FString SlotName;
	
	UPROPERTY()
	TObjectPtr<UItemObject> ItemObject;

	UPROPERTY(NotReplicated)
	TArray<FGameplayAbilitySpecHandle> Abilities;
};

USTRUCT()
struct FOutfitList : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	FOutfitList() : OwnerComponent(nullptr)
	{
	}

	FOutfitList(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent)
	{
	}

	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FAppliedOutfitEntry, FOutfitList>(Entries, DeltaParams, *this);
	}

	UItemObject* AddEntry(const FString& SlotName, UItemObject* ItemObject);
	void RemoveEntry(const FString& SlotName);

private:
	UPROPERTY()
	TArray<FAppliedOutfitEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

USTRUCT(Blueprintable)
struct FOutfitSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Outfit Slot")
	FString SlotName = "Default";
	
	UPROPERTY(VisibleInstanceOnly, Category = "Outfit Slot")
	UItemObject* ArmedObject = nullptr;
	
	FOutfitSlot()
	{
	}

	const FString& GetSlotName() const
	{
		return SlotName;
	}
	
	bool IsArmed() const
	{
		return ArmedObject != nullptr;
	}
};

namespace FItemSystemTags
{
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTag_Item);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTag_Armor);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTag_ArmorBody);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTag_Weapon);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTag_WeaponPrimary);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemTag_WeaponSecondary);
}

UCLASS()
class STALKER_API UItemSystemCore : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UItemObject* GenerateItemObject(UWorld* World, const UItemObject* ItemObject);
	static UItemObject* GenerateItemObject(UWorld* World, const UItemDefinition* Definition,
	                                       const UItemPredictedData* PredictedData);
	static void DestroyItemObject(const UItemObject* ItemObject);

	static UItemObject* GetItemObjectById(const UWorld* World, uint32 ItemId);
	static bool IsItemObjectValid(const UWorld* World, uint32 ItemId);

protected:
	
	
private:
	static void AddItemObjectToGameState(UItemObject* ItemObject);
	static void RemoveItemObjectFromGameState(const UItemObject* ItemObject);
	
	static uint32 LastItemId;
};
