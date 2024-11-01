// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContainerGridWidget.generated.h"

enum class EDragDropOperationResult : uint8;
class UItemsContainer;
class UItemsContainerComponent;
class UItemObject;
class UItemWidget;

USTRUCT()
struct FHoveredItemData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<const UItemObject> ItemRef = nullptr;

	bool bHighlightItem = false;
	
	FIntPoint Tile = FIntPoint::NoneValue;

	FIntPoint Size = FIntPoint::NoneValue;

	bool HasValidData() const
	{
		return ItemRef.IsValid() && bHighlightItem;
	}
	
	void Clear()
	{
		ItemRef.Reset();
		bHighlightItem = false;
		Tile = FIntPoint::NoneValue;
		Size = FIntPoint::NoneValue;
	}
};

USTRUCT()
struct FDraggedItemData
{
	GENERATED_USTRUCT_BODY()

	TWeakObjectPtr<const UItemObject> ItemRef = nullptr;

	FIntPoint Tile = FIntPoint::NoneValue;

	FIntPoint SourceTile = FIntPoint::NoneValue;

	void Clear()
	{
		ItemRef.Reset();
		Tile = FIntPoint::NoneValue;
		SourceTile = FIntPoint::NoneValue;
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

	TWeakObjectPtr<UItemsContainerComponent> ItemsContainerComponentRef;

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                          const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	                          const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                              UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;

	void ClearChildrenItems();
	
	void UpdateItemsMap();
	void UpdateGrid();

	void OnItemAdded(UItemObject* ItemObject);
	void OnItemRemoved(UItemObject* ItemObject);

	void OnItemMouseEnter(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent, UItemObject* HoverItem);
	void OnItemMouseLeave(UItemObject* HoverItem);
	
	void OnBeginDragOperation(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UItemObject* ItemObject);
	void OnNotifiedAboutDropOperation(UItemObject* DraggedItem, EDragDropOperationResult OperationResult);
	
	uint32 FindAvailableRoom(const UItemObject* ItemObject, bool& bFound);
	
	void FillRoom(uint32 ItemId, const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width);
	void ClearRoom(uint32 ItemId);

	bool IsStackableRoom(const UItemObject* ItemObject, uint32 Index);
	bool IsAvailableRoom(const UItemObject* ItemObject, uint32 Index);
	
	bool CheckRoom(const UItemObject* ItemObject, uint32 Index);
	bool IsRoomValid(const FIntPoint& Tile, const FIntPoint& ItemSize, uint8 Width);
	bool IsItemSizeValid(const FIntPoint& ItemSize, uint8 Width);
	bool IsTileFilled(uint32 Index);

	UItemWidget* CreateItemWidget(UItemObject* ItemObject, const FVector2D& PositionOnGrid);

	static FIntPoint TileFromIndex(uint32 Index, uint8 Width);
	static uint32 IndexFromTile(const FIntPoint& Tile, uint8 Width);

	static bool IsMouseOnTile(float MousePosition);

	static FIntPoint GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition);
	static FIntPoint GetTileFromMousePosition(const FGeometry& InGeometry, const FVector2D& ScreenSpacePosition,
	                                          const UItemObject* ItemObject);
	
private:
	const uint8 Columns = 8;
	
	FHoveredItemData HoveredData;
	
	FDraggedItemData DraggedData;

	TArray<uint32> ItemsSlots;
	
	TMap<uint32, FIntPoint> ItemsMap;
};
