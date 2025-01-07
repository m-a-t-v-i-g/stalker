// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UsableInterface.h"
#include "UObject/Object.h"
#include "ItemObject.generated.h"

UENUM()
enum class EItemMode : uint8
{
	Grounded,
	Collected,
	Equipped
};

USTRUCT()
struct FItemInstanceData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditInstanceOnly, Category = "Item")
	uint32 ItemId = 0;
	
	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ClampMin = "1"))
	uint16 Amount = 1;
	
	UPROPERTY(EditInstanceOnly, Category = "Item", meta = (ForceUnits = "%"))
	float Endurance = 100.0f;

	UPROPERTY(EditInstanceOnly, Category = "Item")
	EItemMode Mode = EItemMode::Grounded;
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
	
	UPROPERTY(EditAnywhere, Category = "Spoiling")
	TMap<FGameplayTag, float> SpoilModifiers;
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
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_ItemData", Category = "Item")
	FItemInstanceData ItemData;

	TDelegate<void(const FItemInstanceData&, const FItemInstanceData&)> OnItemDataChangedDelegate;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);
	
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
								 const UItemPredictedData* PredictedData);
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance);

	void UpdateItemInstance(const FItemInstanceData& PrevItemData);
	
protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Item")
	TObjectPtr<const UItemDefinition> ItemDefinition;

	UFUNCTION()
	void OnRep_ItemData(const FItemInstanceData& PrevItemData);
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
	UPROPERTY(VisibleAnywhere, Replicated, Category = "Definition")
	TObjectPtr<const UItemDefinition> ItemDefinition;

	TMulticastDelegate<void(float)> OnEnduranceChangeDelegate;
	TMulticastDelegate<void(uint32)> OnAmountChangeDelegate;
	TMulticastDelegate<void(EItemMode)> OnModeChangeDelegate;
	
#pragma region Replication
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

#pragma endregion Replication

#pragma region Behavior
	
	virtual void Use_Implementation(UObject* Source) override;

	virtual void InitItem(const uint32 ItemId, const UItemObject* ItemObject);
	virtual void InitItem(const uint32 ItemId, const UItemDefinition* Definition, const UItemPredictedData* PredictedData);

	void OnItemInstanceDataChanged(const FItemInstanceData& ItemData, const FItemInstanceData& PrevItemData);
	
	void BindItemActor(AItemActor* BindItem);
	virtual void OnBindItemActor();

	void UnbindItemActor();
	virtual void OnUnbindItemActor(AItemActor* PrevItemActor);
	
	void SetAmount(uint32 Amount);
	void AddAmount(uint32 Amount);
	void RemoveAmount(uint32 Amount);
	void UpdateAmount(uint32 NewAmount, uint32 PrevAmount);
	virtual void OnAmountUpdated(uint32 NewAmount, uint32 PrevAmount);

	void RecoveryEndurance();
	void SpoilEndurance(const FGameplayTag& DamageTag, float DamageValue);
	void UpdateEndurance(float NewEndurance, float PrevEndurance);
	virtual void OnEnduranceUpdated(float NewEndurance, float PrevEndurance);

	virtual bool IsInteractable() const;
	virtual bool CanStackItem(const UItemObject* OtherItem) const;
	
	virtual bool IsCorrespondsTo(const UItemObject* OtherItemObject) const;
	virtual bool HasBoundActor() const;
	
	FORCEINLINE uint32 GetItemId() const;
	FORCEINLINE uint16 GetAmount() const;
	FORCEINLINE float GetEndurance() const;
	FORCEINLINE EItemMode GetItemMode() const;

#pragma endregion Behavior

#pragma region Mode
	
	virtual void SetGrounded();
	virtual void SetCollected();
	virtual void SetEquipped();

	void UpdateMode(EItemMode NewMode, EItemMode PrevMode);
	virtual void OnModeUpdated(EItemMode NewMode, EItemMode PrevMode);
	
	virtual bool IsGrounded() const;
	virtual bool IsCollected() const;
	virtual bool IsEquipped() const;
	
#pragma endregion Mode

#pragma region Static Data

	FORCEINLINE const UItemDefinition* GetDefinition() const;
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
	FORCEINLINE bool IsSpoiling() const;
	FORCEINLINE uint32 GetStackAmount() const;
	FORCEINLINE const TMap<FGameplayTag, float>& GetSpoilModifiers() const;

#pragma endregion Static Data

	FORCEINLINE AItemActor* GetBoundActor() const;

	template <class T>
	T* GetBoundActor() const
	{
		return Cast<T>(GetBoundActor());
	}

	FORCEINLINE UItemInstance* GetItemInstance() const;

	template <class T>
	T* GetItemInstance() const
	{
		return Cast<T>(GetItemInstance());
	}

	FTimerManager& GetWorldTimerManager() const;

protected:
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_ItemInstance", Category = "Instance Data")
	TObjectPtr<UItemInstance> ItemInstance;

	UFUNCTION()
	void OnRep_ItemInstance(UItemInstance* PrevItemInstance);

private:
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_BoundItem", Category = "Instance Data")
	TObjectPtr<AItemActor> BoundItemActor;
	
	UFUNCTION()
	void OnRep_BoundItem(AItemActor* PrevItemActor);
};
