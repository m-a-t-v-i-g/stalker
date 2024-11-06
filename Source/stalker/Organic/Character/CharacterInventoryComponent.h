// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlotContainerInterface.h"
#include "Components/InventoryComponent.h"
#include "CharacterInventoryComponent.generated.h"

class UEquipmentSlot;

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterInventoryComponent : public UInventoryComponent, public ISlotContainerInterface
{
	GENERATED_BODY()

public:
	UCharacterInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void BeginPlay() override;
	
	virtual void EquipSlot(const FString& SlotName, UItemObject* ItemObject) override;

	virtual void UnequipSlot(const FString& SlotName) override;
	
	virtual bool CanEquipItemAtSlot(const FString& SlotName, UItemObject* ItemObject) override;
	
	virtual UEquipmentSlot* FindEquipmentSlot(const FString& SlotName) const override;
	
	UFUNCTION(Server, Reliable)
	void ServerEquipSlot(const FString& SlotName, UItemObject* ItemObject);
	
	UFUNCTION(Server, Reliable)
	void ServerUnequipSlot(const FString& SlotName);
	
	void TryEquipItem(UItemObject* BoundObject);

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Equipment")
	TArray<UEquipmentSlot*> EquipmentSlots;
};
