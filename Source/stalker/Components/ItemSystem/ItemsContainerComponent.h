// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemsContainerComponent.generated.h"

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UItemsContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemsContainerComponent();

	void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Properties", Meta = (ExposeOnSpawn = true))
	uint8 Columns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Properties", Meta = (ExposeOnSpawn = true))
	uint8 Rows = 0;

	UPROPERTY(EditInstanceOnly, Category = "Items Container")
	TArray<class UItemObject*> ItemsContainer;

	UPROPERTY(EditInstanceOnly, Category = "Items Container")
	TArray<uint32> ItemsContainerSlots;

	TMap<uint32, FIntPoint> ItemsContainerMap;

public:
	UFUNCTION(BlueprintCallable)
	bool FindAvailablePlace(const UItemObject* ItemObject);
	
	void AddItemAt(const UItemObject* ItemObject, uint32 Index);

	bool CanAddItem(const UItemObject* ItemObject, uint32& Index);
	
	bool FindAvailableRoom(const UItemObject* ItemObject, uint32& Index);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);

	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, int Width);
	
	static void FillRoom(TArray<uint32>& Slots, uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize,
	                     uint8 Columns);

	static bool IsRoomValid(TArray<uint32>& Slots, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Columns,
	                        uint8 Rows);
	
	static bool IsTileValid(const FIntPoint& Tile, uint8 Columns, uint8 Rows);
	static bool IsTileFilled(const TArray<uint32>& Slots, uint32 Index);
};
