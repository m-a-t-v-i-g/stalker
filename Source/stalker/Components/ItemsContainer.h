// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemObject.h"
#include "UObject/Object.h"
#include "ItemsContainer.generated.h"

class UItemDefinition;
class UItemPredictedData;

DECLARE_MULTICAST_DELEGATE_OneParam(FContainerUpdateDelegate, UItemObject*);

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UItemsContainer : public UObject
{
	GENERATED_BODY()

public:
	FContainerUpdateDelegate OnItemAdded;
	FContainerUpdateDelegate OnItemRemoved;
	
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddStartingData();
	
	bool FindAvailablePlace(UItemObject* ItemObject);

	bool StackItem(UItemObject* SourceItem, UItemObject* TargetItem);
	
	bool AddItem(UItemObject* ItemObject);

	void SplitItem(UItemObject* ItemObject);
	
	bool RemoveItem(UItemObject* ItemObject);

	bool SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount);

	void MoveItemToOtherContainer(UItemObject* ItemObject, UItemsContainer* OtherContainer);

	bool Contains(const UItemObject* ItemObject) const;

	UItemObject* FindItemById(uint32 ItemId) const;

	TArray<UItemObject*> GetItems() const { return ItemsContainer; }
	
protected:
	UPROPERTY(EditAnywhere, Category = "Items Container")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Category = "Items Container")
	TArray<FItemStartingData> StartingData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_ItemsContainer", Category = "Items Container")
	TArray<UItemObject*> ItemsContainer;

	UItemObject* FindAvailableStack(const UItemObject* ItemObject) const;

	UFUNCTION()
	void OnRep_ItemsContainer(TArray<UItemObject*> PrevContainer);
};
