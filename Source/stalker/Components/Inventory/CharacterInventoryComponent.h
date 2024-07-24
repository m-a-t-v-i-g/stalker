// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "CharacterInventoryComponent.generated.h"

USTRUCT(Blueprintable)
struct FEquipmentSlotStartingData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Starting Data")
	FDataTableRowHandle ItemRow;

	bool IsValid() const
	{
		return !ItemRow.IsNull();
	}
};

USTRUCT(Blueprintable)
struct FEquipmentSlotSpec
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FGameplayTagContainer CategoryTags;
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Equipment Slot")
	FEquipmentSlotStartingData StartingData;
};

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterInventoryComponent : public UInventoryComponent
{
	GENERATED_BODY()

public:
	UCharacterInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void InitializeContainer() override;

protected:
	UPROPERTY(EditDefaultsOnly, DisplayName = "Equipment Slots", Category = "Equipment")
	TArray<FEquipmentSlotSpec> EquipmentSlotSpecs;

private:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Equipment Slots")
	TArray<class UEquipmentSlot*> EquipmentSlots;
	
public:
	UEquipmentSlot* FindEquipmentSlotByName(const FString& SlotName) const;
};
