// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemsContainerGridWidget.generated.h"

UCLASS()
class STALKER_API UItemsContainerGridWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> GridCanvas;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> GridSizeBox;
	
	TWeakObjectPtr<class UItemsContainerComponent> ItemsContainerComponent;

	TMap<const class UItemObject*, class UInteractiveItemWidget*> ContainerItems;

public:
	void SetupItemsContainerGrid(UItemsContainerComponent* NewItemsContainer);
	
	void OnItemAddedToContainer(const UItemObject* ItemObject, FIntPoint Tile);
	void OnItemRemovedFromContainer(const UItemObject* ItemObject);

protected:
	void SetupSize();
};
