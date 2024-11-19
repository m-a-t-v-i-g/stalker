// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UPlayerInventoryManagerComponent;
class UItemsContainer;
class UInventoryGridWidget;

UCLASS()
class STALKER_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetupInventory(UPlayerInventoryManagerComponent* PlayerInventoryManager, UItemsContainer* ItemsContainer);
	void ClearInventory();

	UInventoryGridWidget* GetInventoryGridWidget() const { return ItemsGrid; }
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> ItemsGrid;
};
