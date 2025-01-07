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

	void SetItemsContainer(UItemsContainer* InItemsContainer);

	void FindAvailablePlace(UItemObject* ItemObject);
	void StackItem(UItemObject* SourceItem, UItemObject* TargetItem);
	void AddItem(UItemObject* ItemObject);
	void SplitItem(UItemObject* ItemObject);
	void RemoveItem(UItemObject* ItemObject);
	void SubtractOrRemoveItem(UItemObject* ItemObject, uint16 Amount);

	bool CanAddItem(const UItemDefinition* ItemDefinition) const;
	
	UItemObject* FindItemById(uint32 ItemId) const;
	UItemObject* FindItemByDefinition(const UItemDefinition* Definition) const;

	bool IsAuthority() const;
	
	FORCEINLINE UItemsContainer* GetItemsContainer() const { return ItemsContainerRef; }

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "Inventory")
	TObjectPtr<UItemsContainer> ItemsContainerRef;
};
