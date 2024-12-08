// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UEquipmentSlot;
class UItemObject;

USTRUCT(BlueprintType)
struct FCharacterFastUseSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fast Use Slot")
	uint8 SlotId = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fast Use Slot")
	FGameplayTag CategoryTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fast Use Slot")
	TObjectPtr<UItemObject> ItemObject;

	friend uint8 GetTypeHash(const FCharacterFastUseSlot& FastUseSlot)
	{
		return GetTypeHash(FastUseSlot.SlotId);
	}
};

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentComponent();
	
	virtual void BeginPlay() override;

	void EquipSlot(const FString& SlotName, UItemObject* ItemObject);
	void UnequipSlot(const FString& SlotName);
	
	bool IsAuthority() const;

	bool CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject);
	
	UEquipmentSlot* FindEquipmentSlot(const FString& SlotName) const;

	TArray<UEquipmentSlot*> GetEquipmentSlots() const { return EquipmentSlots; }
	
	TArray<UItemObject*> GetAllEquippedItems() const;
	
protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Equipment")
	TArray<UEquipmentSlot*> EquipmentSlots;
	
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TArray<FCharacterFastUseSlot> FastUseSlots;
};
