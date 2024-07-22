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
	
	TWeakObjectPtr<class UItemsContainerComponent> ItemsContainerRef;

private:
	TMap<const uint32, class UInteractiveItemWidget*> ItemWidgetsMap;

public:
	void SetupContainerGrid(UItemsContainerComponent* OwnContainerComp);
	
	void OnItemAddedToContainer(const class UItemObject* ItemObject, FIntPoint Tile);
	void OnItemRemovedFromContainer(const UItemObject* ItemObject);

protected:
	void SetupSize();
};
