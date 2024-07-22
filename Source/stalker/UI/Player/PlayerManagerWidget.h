// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerManagerWidget.generated.h"

UCLASS()
class STALKER_API UPlayerManagerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> PlayerInventory;

public:
	void InitializeInventory(class UItemsContainerComponent* ItemsContainerComponent);
};
