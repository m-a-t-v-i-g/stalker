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
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                          const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                              UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                               UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> GridCanvas;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> GridSizeBox;
	
	TWeakObjectPtr<class UItemsContainerComponent> ItemsContainerRef;

private:
	TMap<const uint32, class UInteractiveItemWidget*> ItemWidgetsMap;

	FIntPoint DraggedTile;

	bool bDrawDropLocation = false;
	
public:
	void SetupContainerGrid(UItemsContainerComponent* OwnContainerComp);
	
	void OnItemAddedToContainer(class UItemObject* ItemObject, FIntPoint Tile);
	void OnItemRemovedFromContainer(UItemObject* ItemObject);

protected:
	void OnCompleteDragOperation(UItemObject* DraggedItem);
	
	void SetupSize();
	
	static bool IsMouseOnTile(float MousePosition);
};
