// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "InventoryManagerComponent.generated.h"

class UInventoryComponent;
class UItemsContainer;
class UEquipmentSlot;
class UItemObject;
class AStalkerCharacter;

UCLASS()
class STALKER_API UInventoryManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void SetupInventoryManager(APawn* InPawn, AController* InController);
	virtual void ResetInventoryManager();

	void AddReplicatedContainer(UItemsContainer* Container);
	void RemoveReplicatedContainer(UItemsContainer* Container);
	void AddReplicatedEquipmentSlot(UEquipmentSlot* EquipmentSlot);
	void RemoveReplicatedEquipmentSlot(UEquipmentSlot* EquipmentSlot);

	void StackItem(UItemsContainer* Container, UObject* Source, UItemObject* SourceItem, UItemObject* TargetItem);
	void SplitItem(UItemsContainer* Container, UItemObject* ItemObject);
	void AddItem(UItemsContainer* Container, UObject* Source, UItemObject* ItemObject);
	void RemoveItem(UItemsContainer* Container, UItemObject* ItemObject);
	void MoveItemToOtherContainer(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject);
	
	void TryEquipItem(UObject* Source, UItemObject* ItemObject);
	void EquipSlot(UEquipmentSlot* EquipmentSlot, UObject* Source, UItemObject* ItemObject);
	void UnequipSlot(UEquipmentSlot* EquipmentSlot);
	void MoveItemFromEquipmentSlotToContainer(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container);

protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory Manager")
	TArray<UItemsContainer*> ReplicatedContainers;

	UPROPERTY(EditAnywhere, Replicated, Category = "Inventory Manager")
	TArray<UEquipmentSlot*> ReplicatedEquipmentSlots;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStackItem(UItemsContainer* Container, UObject* Source, UItemObject* SourceItem, UItemObject* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSplitItem(UItemsContainer* Container, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(UItemsContainer* Container, UObject* Source, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(UItemsContainer* Container, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemToOtherContainer(UItemsContainer* FromContainer, UItemsContainer* ToContainer, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryEquipItem(UObject* Source, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipSlot(UEquipmentSlot* EquipmentSlot, UObject* Source, UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUnequipSlot(UEquipmentSlot* EquipmentSlot);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemFromEquipmentSlotToContainer(UEquipmentSlot* EquipmentSlot, UItemsContainer* Container);

	void RemoveItemFromSource(UObject* Source, UItemObject* ItemObject);
	
	void OnPossibleInteractionAdd(AActor* TargetActor);
	void OnPossibleInteractionRemove(AActor* TargetActor);
	
	bool IsItemObjectValid(uint32 ItemId) const;

	UItemObject* GetItemObjectById(uint32 ItemId) const;

private:
	UPROPERTY()
	TObjectPtr<APawn> PawnRef;

	UPROPERTY()
	TObjectPtr<AController> ControllerRef;
	
	TObjectPtr<UItemsContainer> OwnItemsContainer;
	TArray<UEquipmentSlot*> OwnEquipmentSlots;
};
