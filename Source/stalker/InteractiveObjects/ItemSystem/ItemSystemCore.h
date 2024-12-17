// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemsLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemSystemCore.generated.h"

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

USTRUCT(Blueprintable)
struct FArmorStaticDataTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Armor")
	TSubclassOf<UGameplayEffect> Effect;
	
	UPROPERTY(EditAnywhere, Category = "Armor")
	TObjectPtr<UCurveFloat> ProtectionFactor;
	
	UPROPERTY(EditAnywhere, Category = "Armor")
	TMap<FGameplayTag, float> ProtectionModifiers;
};

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
