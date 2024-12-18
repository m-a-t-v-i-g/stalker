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

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFindAvailablePlace(UItemsContainer* Container, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStackItem(UItemsContainer* Container, UItemObject* SourceItem, UItemObject* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(UItemsContainer* Container, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSplitItem(UItemsContainer* Container, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(UItemsContainer* Container, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSubtractOrRemoveItem(UItemsContainer* Container, UItemObject* ItemObject, uint16 Amount);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemToOtherContainer(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryEquipItem(UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipSlot(UEquipmentSlot* EquipmentSlot, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUnequipSlot(UEquipmentSlot* EquipmentSlot);

	UFUNCTION(Server, Reliable, WithValidation)
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
