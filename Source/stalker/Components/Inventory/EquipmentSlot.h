// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterInventoryComponent.h"
#include "GameplayTagContainer.h"
#include "EquipmentSlot.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSlotChangedSignature, const FGameplayTag&, UItemObject*)

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
	
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slot")
	FGameplayTag ItemTag;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slot")
	TWeakObjectPtr<UItemObject> BoundObject;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slot")
	bool bAvailable = true;

public:
	FOnSlotChangedSignature OnSlotChanged;
	
	void SetupSlot(const FEquipmentSlotSpec* EquipmentSlotSpec);

	bool EquipSlot(const FGameplayTag& InItemTag, UItemObject* BindObject);
	void UnEquipSlot();

	bool CanEquipItem(const UItemObject* ItemObject) const;
	
	bool IsEquipped() const { return ItemTag.IsValid() && BoundObject != nullptr; }
	
	const FString& GetSlotName() const { return SlotName; }

	const FGameplayTag& GetItemTag() const { return ItemTag; }
	UItemObject* GetBoundObject() const { return BoundObject.Get(); }

	void UpdateSlot() const;
};
