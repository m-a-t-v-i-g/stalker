// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemObject.h"
#include "UObject/Object.h"
#include "ItemsContainer.generated.h"

class UItemDefinition;
class UItemPredictedData;

USTRUCT()
struct FItemsContainerChangeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<UItemObject*> AddedItems;

	UPROPERTY()
	TArray<UItemObject*> RemovedItems;

	FItemsContainerChangeData()
	{
	}
	
	FItemsContainerChangeData(const TArray<UItemObject*>& NewItems, const TArray<UItemObject*>& OldItems)
	{
		AddedItems = NewItems;
		RemovedItems = OldItems;
	}
};

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UItemsContainer : public UObject
{
	GENERATED_BODY()

public:
	TMulticastDelegate<void(const FItemsContainerChangeData&)> OnContainerChangeDelegate;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);
	
	void SetupItemsContainer(FGameplayTagContainer ContainerTags);
	
	void AddStartingData();
	
	bool FindAvailablePlace(UItemObject* ItemObject);
	bool StackItem(UItemObject* SourceItem, UItemObject* TargetItem);
	void SplitItem(UItemObject* ItemObject);
	void AddItem(UItemObject* ItemObject);
	void RemoveItem(UItemObject* ItemObject);
	bool SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount);
	bool CanAddItem(const UItemDefinition* ItemDefinition) const;
	
	bool Contains(const UItemObject* ItemObject) const;

	UItemObject* FindAvailableStack(const UItemObject* ItemObject) const;
	UItemObject* FindItemById(uint32 ItemId) const;
	UItemObject* FindItemByDefinition(const UItemDefinition* Definition) const;

	TArray<UItemObject*> GetItems() const { return Items; }
	
protected:
	UPROPERTY(EditAnywhere, Category = "Items Container")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Category = "Items Container")
	TArray<FItemStartingData> StartingData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_Items", Category = "Items Container")
	TArray<UItemObject*> Items;

	UFUNCTION()
	void OnRep_Items(TArray<UItemObject*> PrevContainer);
};
