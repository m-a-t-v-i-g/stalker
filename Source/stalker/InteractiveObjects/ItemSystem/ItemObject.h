// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UsableInterface.h"
#include "Items/ItemsLibrary.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

UCLASS()
class STALKER_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Class")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<class AItemActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class UItemInstance> ItemInstanceClass;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<class UItemObject> ObjectClass;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Description")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Description")
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditAnywhere, Category = "Properties")
	FIntPoint Size;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bUsable = false;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bDroppable = true;

	UPROPERTY(EditAnywhere, Category = "Properties")
	bool bStackable = false;
	
	UPROPERTY(EditAnywhere, Category = "Properties", meta = (ClampMin = "0.0", ForceUnits = "kg"))
	float Weight = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Properties", meta = (ClampMin = "1"))
	int StackAmount = 1;
};

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UItemPredictedData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (ClampMin = "1"))
	uint16 Amount = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Item", meta = (ClampMin = "0.0", ClampMax = "100.0", ForceUnits = "%"))
	float Endurance = 100.0f;
};

UCLASS()
class STALKER_API UItemInstance : public UObject
{
	GENERATED_BODY()

public:
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemPredictedData* PredictedData);
	
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance);
	
	UPROPERTY(EditInstanceOnly, Category = "Item")
	uint32 ItemId = 1;
	
	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ClampMin = "1"))
	uint16 Amount = 1;
	
	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ForceUnits = "%"))
	float Endurance = 100.0f;

protected:
	UPROPERTY(EditInstanceOnly, Category = "Item")
	TWeakObjectPtr<const UItemDefinition> ItemDefinition;
};

UCLASS(BlueprintType, Blueprintable)
class STALKER_API UItemObject : public UObject, public IUsableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Definition")
	TObjectPtr<const UItemDefinition> ItemDefinition;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Use_Implementation(UObject* Source) override;
	
	virtual void InitItem(const uint32 ItemId, const FDataTableRowHandle& RowHandle);
	virtual void InitItem(const uint32 ItemId, const UItemObject* ItemObject);

	virtual void InitItem(const uint32 ItemId, const UItemDefinition* Definition, const UItemPredictedData* PredictedData);
	
	void BindItem(AItemActor* BindItem);
	virtual void OnBindItem();

	void UnbindItem();
	virtual void OnUnbindItem();

	virtual void SetInventoriedMode();
	virtual void SetEquippedMode();

	void SetAmount(uint32 Amount) const;
	void AddAmount(uint32 Amount) const;
	void RemoveAmount(uint32 Amount) const;

	virtual bool IsSimilar(const UItemObject* OtherItemObject) const;

	FORCEINLINE uint32 GetItemId() const;
	
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
	
	FORCEINLINE AItemActor* GetBoundItem() const { return BoundItem.Get(); }

	template <class T>
	T* GetBoundItem() const
	{
		return Cast<T>(GetBoundItem());
	}

	FORCEINLINE UItemInstance* GetItemInstance() const { return ItemInstance.Get(); }

	template <class T>
	T* GetItemInstance() const
	{
		return Cast<T>(GetItemInstance());
	}

	bool CanStackItem(const UItemObject* OtherItem) const;
	
	/* DEPRECATED */
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
	
	FORCEINLINE FItemParams GetItemParams() const;
	FORCEINLINE const UDataTable* GetItemDataTable() const;
	FORCEINLINE FName GetItemRowName() const;

protected:
	UPROPERTY(EditAnywhere, Category = "Instance Data")
	TObjectPtr<UItemInstance> ItemInstance;

	/* DEPRECATED. */
	UPROPERTY(Replicated)
	FItemParams ItemParams;

	/* DEPRECATED. */
	UPROPERTY(Replicated)
	const UDataTable* ItemDataTable;

	/* DEPRECATED. */
	UPROPERTY(Replicated)
	FName ItemRowName;

	/* DEPRECATED. */
	UPROPERTY(Replicated)
	bool bStackable = false;

	virtual void SetupItemProperties();
	
	UFUNCTION()
	void OnRep_BoundItem();

private:
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_BoundItem", Category = "Instance Data")
	TObjectPtr<AItemActor> BoundItem;
};
