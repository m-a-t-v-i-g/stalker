// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ItemsContainer.generated.h"

class UItemDefinition;
class UItemPredictedData;
class UItemObject;

USTRUCT(BlueprintType, Blueprintable)
struct FItemStartingData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Starting Data")
	TObjectPtr<const UItemDefinition> Definition;
	
	UPROPERTY(EditAnywhere, Category = "Starting Data")
	bool bUsePredictedData = false;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Starting Data", meta = (EditCondition = "bUsePredictedData"))
	TObjectPtr<UItemPredictedData> PredictedData;
	
	bool IsValid() const
	{
		return Definition != nullptr;
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FContainerUpdateDelegate, UItemObject*);

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UItemsContainer : public UObject
{
	GENERATED_BODY()

public:
	FContainerUpdateDelegate OnItemAdded;
	FContainerUpdateDelegate OnItemRemoved;
	
	void AddStartingData();
	
	bool FindAvailablePlace(UItemObject* ItemObject);

	bool StackItem(UItemObject* SourceItem, UItemObject* TargetItem);
	
	bool AddItem(UItemObject* ItemObject);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Items Container")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Category = "Items Container")
	TArray<FItemStartingData> StartingData;

	UPROPERTY(EditInstanceOnly, Category = "Items Container")
	TArray<UItemObject*> ItemsContainer;

	UItemObject* FindAvailableStack(const UItemObject* ItemObject) const;
};
