// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Library/Items/ItemsLibrary.h"
#include "ItemsContainerComponent.generated.h"

class UItemObject;

USTRUCT(BlueprintType, Blueprintable)
struct FItemsContainerStartingData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Starting Data")
	FDataTableRowHandle ItemRow;

	UPROPERTY(EditAnywhere, Category = "Starting Data")
	FItemParams ItemParams;

	UPROPERTY(EditAnywhere, Category = "Starting Data", meta = (ClampMin = "1"))
	int Quantity = 1;

	bool IsValid() const
	{
		return !ItemRow.IsNull();
	}
};

DECLARE_MULTICAST_DELEGATE(FOnItemsContainerUpdatedSignature);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UItemsContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemsContainerComponent();

protected:
	UPROPERTY(EditAnywhere, Category = "Items Container")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(VisibleAnywhere, Category = "Items Container")
	uint8 Columns = 9;

	UPROPERTY(EditAnywhere, Category = "Items Container")
	uint8 Capacity = 70;

	UPROPERTY(EditInstanceOnly, Category = "Items Container")
	TArray<UItemObject*> ItemsContainer;

	UPROPERTY(VisibleInstanceOnly, Category = "Items Container")
	TArray<uint32> ItemsSlots;

	UPROPERTY(VisibleInstanceOnly, Category = "Items Container")
	TMap<uint32, FIntPoint> ItemsMap;

	UPROPERTY(EditAnywhere, Category = "Items Container")
	TArray<FItemsContainerStartingData> StartingData;

public:
	FOnItemsContainerUpdatedSignature OnItemsContainerUpdated;
	
	virtual void InitializeContainer();
	
	virtual void AddStartingData();
	
	bool FindAvailablePlace(UItemObject* ItemObject);

	bool TryAddItem(UItemObject* ItemObject);

	void StackItemAt(const UItemObject* ItemObject, uint32 Index);
	void AddItemAt(UItemObject* ItemObject, uint32 Index);
	void DragItem(const UItemObject* ItemObject);
	void RemoveItem(UItemObject* ItemObject);
	void UpdateItemsMap();

	bool CanStackAtIndex(const UItemObject* ItemObject, uint32 RoomIndex);
	bool CanAddItem(const UItemObject* ItemObject, uint32& FindIndex);

	bool FindAvailableRoom(const UItemObject* ItemObject, uint32& FindIndex);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);

	UItemObject* FindItemById(uint32 ItemId) const;
	
	FORCEINLINE uint8 GetColumns() const { return Columns; }
	
	FORCEINLINE TArray<UItemObject*> GetItemsContainer() const { return ItemsContainer; }
	FORCEINLINE TArray<uint32> GetItemsSlots() const { return ItemsSlots; }
	FORCEINLINE TMap<uint32, FIntPoint> GetItemsMap() const { return ItemsMap; }
	
	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, int Width);
	
	static void FillRoom(TArray<uint32>& Slots, uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize,
	                     uint8 Columns);

	static bool IsRoomValid(TArray<uint32>& Slots, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Columns);
	
	static bool IsItemSizeValid(const FIntPoint& ItemSize, uint8 Columns);
	static bool IsTileFilled(const TArray<uint32>& Slots, uint32 Index);
};
