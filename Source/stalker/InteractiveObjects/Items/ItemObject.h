// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UsableInterface.h"
#include "Items/ItemsLibrary.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

UCLASS(BlueprintType, Blueprintable)
class STALKER_API UItemObject : public UObject, public IUsableInterface
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Use_Implementation(UObject* Source) override;
	
protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Instance Data")
	FItemParams ItemParams;
	
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Instance Data")
	const UDataTable* ItemDataTable;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Instance Data")
	FName ItemRowName;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Instance Data")
	bool bStackable = false;
	
public:
	virtual void InitItem(const uint32 ItemId, const FItemData& ItemData);
	virtual void InitItem(const uint32 ItemId, const UItemObject* ItemObject);
	
protected:
	virtual void SetupItemProperties();
	
public:
	virtual void SetInventoriedMode();
	virtual void SetEquippedMode();

	void SetAmount(uint32 Amount);
	void AddAmount(uint32 Amount);
	void RemoveAmount(uint32 Amount);

	void SetStackable(bool bNewStackable) { bStackable = bNewStackable; }
	
	virtual bool IsSimilar(const UItemObject* OtherItemObject) const;
	
	FORCEINLINE uint32 GetItemId() const;
	FORCEINLINE FItemParams GetItemParams() const;

	FORCEINLINE const UDataTable* GetItemDataTable() const;
	FORCEINLINE FName GetItemRowName() const;

	FORCEINLINE FGameplayTag GetItemTag() const;
	FORCEINLINE UClass* GetActorClass() const;
	FORCEINLINE UClass* GetObjectClass() const;

	FORCEINLINE FText GetItemName() const;
	FORCEINLINE FText GetItemDesc() const;
	FORCEINLINE UTexture2D* GetThumbnail() const;
	
	FORCEINLINE FIntPoint GetItemSize() const;

	FORCEINLINE bool IsUsable() const;
	FORCEINLINE bool IsDroppable() const;
	FORCEINLINE bool IsStackable() const;

	FORCEINLINE uint32 GetStackAmount() const;

	FORCEINLINE UStaticMesh* GetPreviewMesh() const;
	
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
