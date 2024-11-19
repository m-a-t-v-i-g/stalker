// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemsContainer;
class UItemDefinition;
class UItemObject;

DECLARE_MULTICAST_DELEGATE(FOnItemsContainerUpdatedSignature);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	FOnItemsContainerUpdatedSignature OnItemsContainerUpdated;

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFindAvailablePlace(UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStackItem(UItemObject* SourceItem, UItemObject* TargetItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSplitItem(UItemObject* ItemObject);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(UItemObject* ItemObject);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItemToOtherContainer(UItemObject* ItemObject, UItemsContainer* OtherContainer);

	bool HasAuthority() const;

	bool CanAddItem(const UItemDefinition* ItemDefinition) const;
	
	UItemObject* FindItemById(uint32 ItemId) const;

	UItemObject* FindItemByDefinition(const UItemDefinition* Definition) const;

	FORCEINLINE UItemsContainer* GetItemsContainer() const { return ItemsContainerRef; }

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Inventory")
	TObjectPtr<UItemsContainer> ItemsContainerRef;

	UItemObject* GetItemObjectById(uint32 ItemId) const;
	
	bool IsItemObjectValid(uint32 ItemId) const;
};
