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

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
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
	void AddAmount(uint32 Amount);

	bool IsSimilar(const UItemObject* OtherItemObject) const;
	
	FORCEINLINE uint32 GetItemId() const;
	FORCEINLINE FItemParams GetItemParams() const;

	FORCEINLINE FName GetItemRowName() const;

	FORCEINLINE FGameplayTag GetItemTag() const;
	FORCEINLINE UClass* GetActorClass() const;
	FORCEINLINE UClass* GetObjectClass() const;

	FORCEINLINE FIntPoint GetItemSize() const;

	FORCEINLINE bool IsUsable() const;
	FORCEINLINE bool IsDroppable() const;
	FORCEINLINE bool IsStackable() const;

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
