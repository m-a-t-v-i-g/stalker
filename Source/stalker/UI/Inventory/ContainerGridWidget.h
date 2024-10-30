// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContainerGridWidget.generated.h"

enum class EDragDropOperationResult : uint8;
class UItemObject;
class UItemsContainer;
class UItemWidget;

USTRUCT()
struct FItemWidgetData
{
	GENERATED_USTRUCT_BODY()

	TObjectPtr<UItemWidget> ItemWidget = nullptr;
	
	FIntPoint Tile = FIntPoint::NoneValue;

	FItemWidgetData() {}

	FItemWidgetData(UItemWidget* Visual, FIntPoint Placement) : ItemWidget(Visual), Tile(Placement)
	{
	}
};

USTRUCT()
struct FHoveredItemData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<UItemObject> HoveredItemRef = nullptr;

	bool bHighlightItem = false;
	
	FIntPoint Tile = FIntPoint::NoneValue;

	bool HasValidData() const
	{
		return HoveredItemRef.IsValid() && bHighlightItem;
	}
	
	void Clear()
	{
		HoveredItemRef.Reset();
		bHighlightItem = false;
		Tile = FIntPoint::NoneValue;
	}
};

USTRUCT()
struct FDraggedItemData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<UItemObject> ItemObject = nullptr;

	TObjectPtr<UItemWidget> ItemWidget = nullptr;

	FIntPoint Tile = FIntPoint::NoneValue;

	uint32 Index = 0;

	FIntPoint ItemSize = FIntPoint::NoneValue;

	void Clear()
	{
		ItemObject.Reset();
		ItemWidget = nullptr;
		Tile = FIntPoint::NoneValue;
		Index = 0;
		ItemSize = FIntPoint::NoneValue;
	}
};

UCLASS()
class STALKER_API UContainerGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupContainerGrid(UItemsContainer* OwnContainer);
	void ClearContainerGrid();
	
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

	void OnItemMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UItemObject* HoverItem);
	void OnItemMouseLeave(UItemObject* HoverItem);
	
	void OnBeginDragOperation(UItemObject* DraggedItem);
	void OnReverseDragOperation(UItemObject* DraggedItem);
	void OnCompleteDragOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult);
	
	uint32 FindAvailableRoom(const UItemObject* ItemObject, bool& bFound);
	
	void FillRoom(uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width);
	void ClearRoom(uint32 ItemId);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);
	bool IsRoomValid(const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width);
	bool IsItemSizeValid(const FIntPoint& ItemSize, uint8 Width);
	bool IsTileFilled(uint32 Index);

	void EmplaceItemOnGrid(const UItemObject* ItemObject);
	
	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, uint8 Width);

	static bool IsMouseOnTile(float MousePosition);

	static FIntPoint GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition);
	
private:
	const uint8 Columns = 8;
	
	FHoveredItemData HoveredData;
	
	FDraggedItemData DraggedData;

	TMap<UItemObject*, FItemWidgetData> ItemsMap;
	
	TArray<uint32> Tiles;
	
	//TMap<uint32, FIntPoint> ItemsMap;
};
