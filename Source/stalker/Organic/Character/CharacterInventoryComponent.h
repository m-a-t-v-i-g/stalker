// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SlotContainerInterface.h"
#include "Components/InventoryComponent.h"
#include "CharacterInventoryComponent.generated.h"

class UEquipmentSlot;

USTRUCT(BlueprintType)
struct FCharacterFastUseSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fast Use Slot")
	uint8 SlotId = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fast Use Slot")
	FGameplayTag CategoryTags;
	
	TWeakObjectPtr<UItemObject> ItemObject;

	friend uint8 GetTypeHash(const FCharacterFastUseSlot& FastUseSlot)
	{
		return GetTypeHash(FastUseSlot.SlotId);
	}
};

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterInventoryComponent : public UInventoryComponent, public ISlotContainerInterface
{
	GENERATED_BODY()

public:
	UCharacterInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void BeginPlay() override;
	
	virtual void EquipSlot(const FString& SlotName, uint32 ItemId) override;
	virtual void UnequipSlot(const FString& SlotName) override;
	
	virtual bool CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject) override;
	
	virtual UEquipmentSlot* FindEquipmentSlot(const FString& SlotName) const override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipSlot(const FString& SlotName, uint32 ItemId);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUnequipSlot(const FString& SlotName);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemFromSlot(const FString& SlotName);
	
	void TryEquipItem(UItemObject* BoundObject);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipFastUseSlot(uint8 SlotId);
	
	void TryUseFastSlot(uint8 SlotId);

	bool IsEquipmentSlotValid(const FString& SlotName) const;
	bool IsFastUseSlotValid(uint8 SlotId) const;
	
protected:
	UPROPERTY(EditAnywhere, Instanced, Replicated, Category = "Equipment")
	TArray<UEquipmentSlot*> EquipmentSlots;

	UPROPERTY(EditAnywhere, Replicated, Category = "Equipment")
	TArray<FCharacterFastUseSlot> FastUseSlots;
};
