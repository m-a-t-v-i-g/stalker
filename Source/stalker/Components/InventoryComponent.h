// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemsContainer;
class UItemObject;

DECLARE_MULTICAST_DELEGATE(FOnItemsContainerUpdatedSignature);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	FOnItemsContainerUpdatedSignature OnItemsContainerUpdated;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	virtual void BeginPlay() override;

	virtual void PreInitializeContainer();
	virtual void PostInitializeContainer();
	
	void StackItem(UItemObject* SourceObject, UItemObject* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(uint32 ItemId);
	
	void SplitItem(UItemObject* ItemObject);
	
	void RemoveItem(UItemObject* ItemObject);
	
	void SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount);
	
	void DropItem(UItemObject* ItemObject);
	
	void UseItem(UItemObject* ItemObject);

	bool HasAuthority() const;

	FORCEINLINE UItemsContainer* GetItemsContainer() const { return ItemsContainerRef; }

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Inventory")
	TObjectPtr<UItemsContainer> ItemsContainerRef;

	UItemObject* GetItemObjectById(uint32 ItemId) const;
	
	bool IsItemObjectExist(uint32 ItemId) const;
};
