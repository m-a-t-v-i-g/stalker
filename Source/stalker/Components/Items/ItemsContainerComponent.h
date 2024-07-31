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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Items Container")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(VisibleAnywhere, Category = "Items Container")
	uint8 Columns = 9;

	UPROPERTY(EditAnywhere, Replicated, Category = "Items Container")
	uint8 Capacity = 70;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Items Container")
	TArray<UItemObject*> ItemsContainer;

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = "OnRep_ItemsSlots", Category = "Items Container")
	TArray<uint32> ItemsSlots;

	UPROPERTY(VisibleInstanceOnly, Category = "Items Container")
	TMap<uint32, FIntPoint> ItemsMap;

	UPROPERTY(EditAnywhere, Category = "Items Container")
	TArray<FItemsContainerStartingData> StartingData;

public:
	FOnItemsContainerUpdatedSignature OnItemsContainerUpdated;

	UFUNCTION(BlueprintCallable)
	virtual void PreInitializeContainer();

	UFUNCTION(BlueprintCallable)
	virtual void PostInitializeContainer();
	
	virtual void AddStartingData();

	bool FindAvailablePlace(UItemObject* ItemObject);
	bool TryStackItem(UItemObject* ItemObject);
	bool TryAddItem(UItemObject* ItemObject);

	void StackItem(UItemObject* SourceObject, UItemObject* TargetItem);
	
	UFUNCTION(Server, Reliable)
	void Server_StackItem(UItemObject* SourceObject, UItemObject* TargetItem);
	
	void StackItemAt(UItemObject* ItemObject, uint32 Index);
	
	UFUNCTION(Server, Reliable)
	void Server_StackItemAt(UItemObject* ItemObject, uint32 Index);
	
	void SplitItem(UItemObject* ItemObject);
	
	UFUNCTION(Server, Reliable)
	void Server_SplitItem(UItemObject* ItemObject);
	
	void AddItemAt(UItemObject* ItemObject, uint32 Index);
	
	UFUNCTION(Server, Reliable)
	void Server_AddItemAt(UItemObject* ItemObject, uint32 Index);
	
	void RemoveItem(UItemObject* ItemObject);
	
	UFUNCTION(Server, Reliable)
	void Server_RemoveItem(UItemObject* ItemObject);
	
	void RemoveAmountFromItem(UItemObject* ItemObject, uint16 Amount);
	
	UFUNCTION(Server, Reliable)
	void Server_RemoveAmountFromItem(UItemObject* ItemObject, uint16 Amount);
	
	void MoveItemToOtherContainer(UItemObject* ItemObject, UItemsContainerComponent* OtherContainer);
	
	UFUNCTION(Server, Reliable)
	void Server_MoveItemToOtherContainer(UItemObject* ItemObject, UItemsContainerComponent* OtherContainer);
	
	void SubtractOrRemoveItem(UItemObject* ItemObject);
	
	void UpdateItemsMap();

	bool CanStackAtRoom(const UItemObject* ItemObject, uint32 RoomIndex);
	bool CanStackAtIndex(const UItemObject* ItemObject, uint32 RoomIndex);
	bool CanAddItem(const UItemObject* ItemObject) const;

	UItemObject* FindAvailableStack(const UItemObject* ItemObject);
	uint32 FindAvailableRoom(const UItemObject* ItemObject);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);

	UItemObject* FindItemById(uint32 ItemId) const;
	
	FORCEINLINE uint8 GetColumns() const { return Columns; }
	
	FORCEINLINE TArray<UItemObject*> GetItemsContainer() const { return ItemsContainer; }
	FORCEINLINE TArray<uint32> GetItemsSlots() const { return ItemsSlots; }
	FORCEINLINE TMap<uint32, FIntPoint> GetItemsMap() const { return ItemsMap; }

protected:
	UFUNCTION()
	void OnRep_ItemsSlots();
	
public:
	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, int Width);
	
	static void FillRoom(TArray<uint32>& Slots, uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize,
	                     uint8 Columns);

	static bool IsRoomValid(TArray<uint32>& Slots, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Columns);
	
	static bool IsItemSizeValid(const FIntPoint& ItemSize, uint8 Columns);
	static bool IsTileFilled(const TArray<uint32>& Slots, uint32 Index);
};
