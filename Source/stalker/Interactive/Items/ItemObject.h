// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Library/Items/ItemsLibrary.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

UCLASS(BlueprintType, Blueprintable)
class STALKER_API UItemObject : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditInstanceOnly, Category = "Instance Data")
	FItemParams ItemParams;
	
	UPROPERTY(EditInstanceOnly, Category = "Instance Data")
	const UDataTable* ItemDataTable;

	UPROPERTY(EditInstanceOnly, Category = "Instance Data")
	FName ItemRowName;
	
public:
	void InitItem(const uint32 ItemId, const FItemData& ItemData);
	
protected:
	virtual void SetupItemProperties();
	
public:
	uint32 GetItemId() const;

	FGameplayTag GetItemTag() const;
	UClass* GetActorClass() const;
	UClass* GetObjectClass() const;

	/*
	FText GetItemName() const;
	FText GetItemDesc() const;
	*/

	FIntPoint GetItemSize() const;

	bool IsUsable() const;
	bool IsDroppable() const;
	bool IsStackable() const;

	uint16 GetMaxStack() const;
	
protected:
	template <typename Struct>
	Struct* GetRow() const
	{
		Struct* Row = nullptr;
		if (auto DataTable = ItemDataTable)
		{
			Row = DataTable->FindRow<Struct>(ItemRowName, "");
		}
		return Row;
	}
};
