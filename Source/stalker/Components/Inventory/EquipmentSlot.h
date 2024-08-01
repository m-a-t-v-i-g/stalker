// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterInventoryComponent.h"
#include "GameplayTagContainer.h"
#include "EquipmentSlot.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSlotChangedSignature, UItemObject*, bool)

UCLASS()
class STALKER_API UEquipmentSlot : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "Equipment Slot")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Replicated, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_EquipmentSlot", Category = "Equipment Slot")
	UItemObject* BoundObject = nullptr;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slot")
	bool bAvailable = true;

public:
	FOnSlotChangedSignature OnSlotChanged;
	
	void SetupSlot(const FEquipmentSlotSpec* EquipmentSlotSpec);

	bool EquipSlot(UItemObject* BindObject);
	void UnEquipSlot();

	bool CanEquipItem(const UItemObject* ItemObject) const;
	
	bool IsEquipped() const { return BoundObject != nullptr; }
	
	const FString& GetSlotName() const { return SlotName; }

	UItemObject* GetBoundObject() const { return BoundObject; }

	void UpdateSlot(bool bModified) const;

protected:
	UFUNCTION()
	void OnRep_EquipmentSlot(UItemObject* PrevItemObject);
};
