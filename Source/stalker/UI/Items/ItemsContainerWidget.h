// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemsContainerWidget.generated.h"

UCLASS()
class STALKER_API UItemsContainerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UItemsContainerGridWidget> ItemsContainerGrid;

public:
	void InitializeContainer(class UItemsContainerComponent* OwnContainerComp);
};
