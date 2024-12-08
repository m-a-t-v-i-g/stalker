// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagerComponent.generated.h"

class AStalkerCharacter;
class UInventoryComponent;
class UEquipmentSlot;
class UItemsContainer;
class UItemObject;
class APlayerCharacter;

UCLASS()
class STALKER_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryManagerComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void SetupInventoryManager(AController* InController, APlayerCharacter* InCharacter);
	virtual void ResetInventoryManager();

	void AddReplicatedContainer(UItemsContainer* Container);
	void RemoveReplicatedContainer(UItemsContainer* Container);
	void AddReplicatedEquipmentSlot(UEquipmentSlot* EquipmentSlot);
	void RemoveReplicatedEquipmentSlot(UEquipmentSlot* EquipmentSlot);

	void OnLootInventory(UInventoryComponent* InventoryComponent);
	void OnStopLootInventory(UInventoryComponent* InventoryComponent);
	
	void ServerFindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject);
	void ServerStackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem);
	void ServerAddItem(UItemsContainer* Container, UItemObject* ItemObject);
	void ServerSplitItem(UItemsContainer* Container, UItemObject* ItemObject);
	void ServerRemoveItem(UItemsContainer* Container, UItemObject* ItemObject);
	void ServerSubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount);
	void ServerMoveItemToOtherContainer(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject);

	void ServerTryEquipItem(UItemObject* ItemObject);
	void ServerEquipSlot(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject);
	void ServerUnequipSlot(UEquipmentSlot* EquipmentSlot);
	void ServerMoveItemFromEquipmentSlot(UEquipmentSlot* EquipmentSlot);
	
	bool IsAuthority() const;

protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory Manager")
	TArray<UItemsContainer*> ReplicatedContainers;

	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory Manager")
	TArray<UEquipmentSlot*> ReplicatedEquipmentSlots;

	bool IsItemObjectValid(uint32 ItemId) const;

	UItemObject* GetItemObjectById(uint32 ItemId) const;

private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;
	
	TObjectPtr<UItemsContainer> OwnItemsContainer;
	TArray<UEquipmentSlot*> OwnEquipmentSlots;
};
