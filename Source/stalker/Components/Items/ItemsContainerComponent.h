// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemsContainerComponent.generated.h"

class UItemObject;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemAddedToContainer, const UItemObject*, FIntPoint);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemRemovedFromContainer, const UItemObject*);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UItemsContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemsContainerComponent();

	virtual void InitializeComponent() override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items Container")
	uint8 Columns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items Container")
	uint8 Rows = 0;

	UPROPERTY(EditInstanceOnly, Category = "Items Container")
	TArray<UItemObject*> ItemsContainer;

	UPROPERTY(VisibleInstanceOnly, Category = "Items Container")
	TArray<uint32> ItemsContainerSlots;

	TMap<uint32, FIntPoint> ItemsContainerMap;

public:
	FOnItemAddedToContainer OnItemAddedToContainer;
	FOnItemRemovedFromContainer OnItemRemovedFromContainer;

	bool FindAvailablePlace(UItemObject* ItemObject);

	bool TryAddItem(UItemObject* ItemObject);
	
	void AddItemAt(UItemObject* ItemObject, uint32 Index);
	void RemoveItem(UItemObject* ItemObject);

	bool CanAddItem(const UItemObject* ItemObject, uint32& FindIndex);
	
	bool FindAvailableRoom(const UItemObject* ItemObject, uint32& FindIndex);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);

	FORCEINLINE uint8 GetColumns() const { return Columns; }
	FORCEINLINE uint8 GetRows() const { return Rows; }
	
	FORCEINLINE TArray<UItemObject*> GetItemsContainer() const { return ItemsContainer; }
	FORCEINLINE TArray<uint32> GetItemsSlots() const { return ItemsContainerSlots; }
	
	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, int Width);
	
	static void FillRoom(TArray<uint32>& Slots, uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize,
	                     uint8 Columns);

	static bool IsRoomValid(TArray<uint32>& Slots, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Columns,
	                        uint8 Rows);
	
	static bool IsTileValid(const FIntPoint& Tile, uint8 Columns, uint8 Rows);
	static bool IsTileFilled(const TArray<uint32>& Slots, uint32 Index);
};
