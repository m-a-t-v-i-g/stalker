// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Library/Items/ItemsLibrary.h"
#include "ItemsContainerGridWidget.generated.h"

class UItemObject;

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
	
	UPROPERTY(EditAnywhere, Category = "Drawing")
	TObjectPtr<class USlateBrushAsset> GridFillingBrush;
	
	UPROPERTY(EditAnywhere, Category = "Drawing")
	FLinearColor GridHighlightColor;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> GridCanvas;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> GridSizeBox;
	
	TWeakObjectPtr<class UItemsContainerComponent> ItemsContainerRef;

private:
	FIntPoint DraggedTile;

	bool bHighlightItem = false;
	
	TWeakObjectPtr<UItemObject> HoveredItem;
	
public:
	FOnContainerItemOperationSignature OnItemWidgetDoubleClick;
	
	void SetupContainerGrid(UItemsContainerComponent* OwnContainerComp);
	void ClearContainerGrid() const;
	
	void OnItemsContainerUpdated();
	
protected:
	void SetupSize();

	void OnItemMouseEnter(UItemObject* HoverItem);
	void OnItemMouseLeave(UItemObject* HoverItem);
	void OnDoubleClick(UItemObject* ClickedItem);

	void OnBeginDragOperation(UItemObject* DraggedItem);
	void OnReverseDragOperation(UItemObject* DraggedItem);
	void OnCompleteDragOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult);
	
	static bool IsMouseOnTile(float MousePosition);
};
