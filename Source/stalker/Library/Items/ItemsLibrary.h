/* Copyright Next Genium Studio. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ItemsLibrary.generated.h"

USTRUCT(BlueprintType)
struct FItemParams
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, meta = (ClampMin = "1"))
	uint32 ItemId = 1;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	uint16 Amount = 1;
};

USTRUCT(BlueprintType, Blueprintable)
struct FItemData
{
	GENERATED_USTRUCT_BODY()

	FDataTableRowHandle ItemRow;
	FItemParams ItemParams;
	
	bool IsValid() const
	{
		return !ItemRow.IsNull();
	}
};

USTRUCT(BlueprintType)
struct FTableRowItems : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Class")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<AActor> ActorClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<class UItemObject> ObjectClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Properties")
	FIntPoint Size = {0, 0};

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bUsable = false;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bDroppable = true;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bStackable = false;

	UPROPERTY(EditAnywhere, Category = "Properties", meta = (ClampMin = "1"))
	uint16 MaxStack = 1;
};

USTRUCT(BlueprintType)
struct FTableRowMedicament : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct FTableRowFood : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT(BlueprintType)
struct FTableRowAmmo : public FTableRowItems
{
	GENERATED_USTRUCT_BODY()
};
