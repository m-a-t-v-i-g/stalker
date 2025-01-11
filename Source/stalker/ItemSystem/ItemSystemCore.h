// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemSystemCore.generated.h"

class UGameplayEffect;
class UItemDefinition;
class UItemPredictedData;
class UItemObject;

USTRUCT(Blueprintable)
struct FOutfitSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Weapon Slot")
	FString SlotName = "Default";
	
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon Slot")
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
