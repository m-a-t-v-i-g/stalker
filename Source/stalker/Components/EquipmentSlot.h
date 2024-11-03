// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemObject.h"
#include "EquipmentSlot.generated.h"

class UItemObject;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSlotChangedSignature, UItemObject*, bool, bool);

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UEquipmentSlot : public UObject
{
	GENERATED_BODY()

public:
	FOnSlotChangedSignature OnSlotChanged;
	
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddStartingData();
	
	bool EquipSlot(UItemObject* BindObject);
	
	void UnEquipSlot();

	void UpdateSlot(bool bModified) const;

	bool CanEquipItem(const UItemObject* ItemObject) const;
	
	bool IsEquipped() const { return BoundObject != nullptr; }
	
	const FString& GetSlotName() const { return SlotName; }

	UItemObject* GetBoundObject() const { return BoundObject; }

protected:
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FItemStartingData StartingData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_EquipmentSlot", Category = "Equipment Slot")
	TObjectPtr<UItemObject> BoundObject;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slot")
	bool bAvailable = true;

	UFUNCTION()
	void OnRep_EquipmentSlot(UItemObject* PrevItemObject);
};
