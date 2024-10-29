// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "ContainerGridWidget.generated.h"

class UItemObject;
class UItemsContainer;
class UItemWidget;

USTRUCT()
struct FItemWidgetData
{
	GENERATED_USTRUCT_BODY()

	UItemWidget* ItemWidget = nullptr;
	
	UCanvasPanelSlot* ItemSlot = nullptr;

	FItemWidgetData() {}

	FItemWidgetData(UItemWidget* Visual, UCanvasPanelSlot* CanvasSlot) : ItemWidget(Visual), ItemSlot(CanvasSlot)
	{
	}
};

UCLASS()
class STALKER_API UContainerGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupContainerGrid(UItemsContainer* OwnContainer);
	void ClearContainerGrid() const;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> GridCanvas;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> GridSizeBox;
	
	UPROPERTY(EditAnywhere, Category = "Drawing")
	TObjectPtr<class USlateBrushAsset> GridFillingBrush;
	
	UPROPERTY(EditAnywhere, Category = "Drawing")
	FLinearColor GridHighlightColor;

	TWeakObjectPtr<UItemsContainer> ItemsContainerRef;

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                          const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                              UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;
	
	void OnItemAdded(UItemObject* ItemObject);
	void OnItemRemoved(UItemObject* ItemObject);

	uint32 FindAvailableRoom(const UItemObject* ItemObject, bool& bFound);
	
	void FillRoom(uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width);
	void ClearRoom(uint32 ItemId);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);
	bool IsRoomValid(const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width);
	bool IsItemSizeValid(const FIntPoint& ItemSize, uint8 Width);
	bool IsTileFilled(uint32 Index);
	
	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, uint8 Width);

	static bool IsMouseOnTile(float MousePosition);
	
private:
	const uint8 Columns = 8;
	
	FIntPoint DraggedTile;

	TMap<UItemObject*, FItemWidgetData> ItemsMap;
	
	TArray<uint32> Tiles;
};
