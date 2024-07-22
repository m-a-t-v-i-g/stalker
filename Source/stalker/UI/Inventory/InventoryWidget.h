// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

UCLASS()
class STALKER_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UItemsContainerGridWidget> ItemsGrid;
	
public:
	void InitializeInventory(class UItemsContainerComponent* ItemsContainerComponent);
};
