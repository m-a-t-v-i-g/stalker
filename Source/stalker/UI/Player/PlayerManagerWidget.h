// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerManagerWidget.generated.h"

class UStalkerAbilityComponent;
class UInventoryComponent;

UCLASS()
class STALKER_API UPlayerManagerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UInventoryWidget> Inventory;

private:
	TWeakObjectPtr<UStalkerAbilityComponent> OwnAbilityComponent;
	TWeakObjectPtr<UInventoryComponent> OwnInventoryComponent;
	
public:
	void InitializeManager(UStalkerAbilityComponent* AbilityComp, UInventoryComponent* InventoryComp);
};
