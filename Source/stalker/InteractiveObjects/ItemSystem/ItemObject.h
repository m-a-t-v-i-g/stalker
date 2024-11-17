// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UsableInterface.h"
#include "Items/ItemsLibrary.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

UENUM()
enum class EItemMode : uint8
{
	Grounded,
	Collected,
	Equipped
};

UCLASS()
class STALKER_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Class")
	FName ScriptName;

	UPROPERTY(EditAnywhere, Category = "Class")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<class AItemActor> ItemActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class UItemInstance> ItemInstanceClass;

	UPROPERTY(EditAnywhere, Category = "Class")
	TSubclassOf<class UItemObject> ItemObjectClass;

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
	UPROPERTY(EditAnywhere, Category = "Item", meta = (ClampMin = "1"))
	uint16 Amount = 1;
	
	UPROPERTY(EditAnywhere, Category = "Item", meta = (ClampMin = "0.0", ClampMax = "100.0", ForceUnits = "%"))
	float Endurance = 100.0f;
};

UCLASS()
class STALKER_API UItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Category = "Item")
	uint32 ItemId = 1;
	
	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ClampMin = "1"))
	uint16 Amount = 1;
	
	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ForceUnits = "%"))
	float Endurance = 100.0f;

	UPROPERTY(EditInstanceOnly, Category = "Item")
	EItemMode Mode = EItemMode::Grounded;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
								 const UItemPredictedData* PredictedData);
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance);
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Item")
	TWeakObjectPtr<const UItemDefinition> ItemDefinition;
};

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

	void Clear()
	{
		Definition = nullptr;
		bUsePredictedData = false;
		PredictedData = nullptr;
	}
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
	
	virtual void InitItem(const uint32 ItemId, const UItemObject* ItemObject);
	virtual void InitItem(const uint32 ItemId, const UItemDefinition* Definition, const UItemPredictedData* PredictedData);
	
	void BindItemActor(AItemActor* BindItem);
	virtual void OnBindItemActor();

	void UnbindItemActor();
	virtual void OnUnbindItemActor(AItemActor* PrevItemActor);

	virtual void SetGrounded();
	virtual void SetCollected();
	virtual void SetEquipped();

	void SetAmount(uint32 Amount) const;
	void AddAmount(uint32 Amount) const;
	void RemoveAmount(uint32 Amount) const;

	virtual bool IsSimilar(const UItemObject* OtherItemObject) const;
	virtual bool IsGrounded() const;
	virtual bool IsCollected() const;
	virtual bool IsEquipped() const;
	virtual bool HasBoundActor() const;
	
	FORCEINLINE uint32 GetItemId() const;
	FORCEINLINE uint16 GetAmount() const;

#pragma region Static Data
	
	FORCEINLINE FName GetScriptName() const;
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

#pragma endregion Static Data
	
	FORCEINLINE AItemActor* GetBoundItem() const { return BoundItemActor; }

	template <class T>
	T* GetBoundItem() const
	{
		return Cast<T>(GetBoundItem());
	}

	FORCEINLINE UItemInstance* GetItemInstance() const { return ItemInstance; }

	template <class T>
	T* GetItemInstance() const
	{
		return Cast<T>(GetItemInstance());
	}

	virtual bool CanCollected() const;
	virtual bool CanStackItem(const UItemObject* OtherItem) const;
	
	FTimerManager& GetWorldTimerManager() const;
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Instance Data")
	TObjectPtr<UItemInstance> ItemInstance;

	UFUNCTION()
	void OnRep_BoundItem(AItemActor* PrevItemActor);

private:
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_BoundItem", Category = "Instance Data")
	TObjectPtr<AItemActor> BoundItemActor;
};
