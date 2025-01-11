// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemObject.h"
#include "EquipmentSlot.generated.h"

class UItemObject;

USTRUCT()
struct FEquipmentSlotChangeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UItemObject* SlotItem = nullptr;

	bool bIsEquipped = false;
	
	FEquipmentSlotChangeData() {}
	
	FEquipmentSlotChangeData(UItemObject* Item, bool bEquipped)
	{
		SlotItem = Item;
		bIsEquipped = bEquipped;
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotChangedSignature, const FEquipmentSlotChangeData&);

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UEquipmentSlot : public UObject
{
	GENERATED_BODY()

public:
	FOnSlotChangedSignature OnSlotChanged;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

	void SetupEquipmentSlot(FString InSlotName, FGameplayTagContainer InSlotTags);
	
	void AddStartingData();
	
	void EquipSlot(UItemObject* BindObject);
	void UnequipSlot();

	bool CanEquipItem(const UItemDefinition* ItemDefinition) const;
	
	bool IsEquipped() const { return BoundObject != nullptr; }
	
	const FString& GetSlotName() const { return SlotName; }

	UItemObject* GetBoundObject() const { return BoundObject; }

protected:
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	TArray<FItemStartingData> StartingData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_BoundObject", Category = "Equipment Slot")
	TObjectPtr<UItemObject> BoundObject;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slot")
	bool bAvailable = true;

	UFUNCTION()
	void OnRep_BoundObject(UItemObject* PrevItemObject);
};
