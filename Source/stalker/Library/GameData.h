// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameData.generated.h"

USTRUCT(BlueprintType)
struct FGameItemSystemData
{
	GENERATED_USTRUCT_BODY()

#pragma region Item System
	
	UPROPERTY(EditDefaultsOnly, Category = "Item System|Configs")
	TSoftObjectPtr<const class UItemBehaviorSet> ItemBehavior;
	
#pragma endregion Item System
};

USTRUCT(BlueprintType)
struct FGameUIData
{
	GENERATED_USTRUCT_BODY()

#pragma region UI
	
	UPROPERTY(EditDefaultsOnly, Category = "UI|Game")
	TSoftClassPtr<class UGameWidget> GameWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI|Inventory")
	TSoftClassPtr<class UInventoryManagerWidget> InventoryWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI|Inventory")
	TSoftClassPtr<class UItemWidget> ItemWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI|Inventory|Grid")
	float TileSize = 75.0f;
	
	UPROPERTY(EditDefaultsOnly, DisplayName = "Filling Brush", Category = "UI|Inventory|Grid")
	TSoftObjectPtr<class USlateBrushAsset> GridFillingBrush;
	
	UPROPERTY(EditDefaultsOnly, DisplayName = "Highlight Color", Category = "UI|Inventory|Grid")
	FLinearColor GridGreenHighlightColor;
	
	UPROPERTY(EditDefaultsOnly, DisplayName = "Highlight Color", Category = "UI|Inventory|Grid")
	FLinearColor GridRedHighlightColor;
	
	UPROPERTY(EditDefaultsOnly, DisplayName = "Highlight Color", Category = "UI|Inventory|Grid")
	FLinearColor GridBlueHighlightColor;
	
#pragma endregion UI
};

UCLASS()
class STALKER_API UGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (ShowOnlyInnerProperties))
	FGameItemSystemData GameItemSystemData;
	
	UPROPERTY(EditDefaultsOnly, meta = (ShowOnlyInnerProperties))
	FGameUIData GameUIData;
	
	static const UGameData* Get(const UObject* WorldContextObject);
};
