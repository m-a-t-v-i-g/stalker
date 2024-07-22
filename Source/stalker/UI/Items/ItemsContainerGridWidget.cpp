// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerGridWidget.h"
#include "InteractiveItemWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Items/ItemsContainerComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Player/PlayerHUD.h"

void UItemsContainerGridWidget::SetupContainerGrid(UItemsContainerComponent* OwnContainerComp)
{
	if (ItemsContainerRef.IsValid())
	{
		ItemsContainerRef->OnItemAddedToContainer.RemoveAll(this);
	}
	
	ItemsContainerRef = OwnContainerComp;
	
	ItemsContainerRef->OnItemAddedToContainer.AddUObject(this, &UItemsContainerGridWidget::OnItemAddedToContainer);
	ItemsContainerRef->OnItemRemovedFromContainer.AddUObject(this, &UItemsContainerGridWidget::OnItemRemovedFromContainer);

	SetupSize();
}

void UItemsContainerGridWidget::OnItemAddedToContainer(const UItemObject* ItemObject, FIntPoint Tile)
{
	if (!IsValid(ItemObject)) return;
	
	if (UInteractiveItemWidget* ItemWidget = CreateWidget<UInteractiveItemWidget>(this, APlayerHUD::StaticInteractiveItemWidgetClass))
	{
		ItemWidget->InitItemWidget(ItemObject, ItemObject->GetItemSize());
		FVector2D WidgetPosition = {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize};

		ItemWidgetsMap.Add(ItemObject->GetItemId(), ItemWidget);
		
		if (UCanvasPanelSlot* CanvasPanelSlot = GridCanvas->AddChildToCanvas(ItemWidget))
		{
			CanvasPanelSlot->SetAutoSize(true);
			CanvasPanelSlot->SetPosition(WidgetPosition);
		}
	}
}

void UItemsContainerGridWidget::OnItemRemovedFromContainer(const UItemObject* ItemObject)
{
	if (auto FindWidget = ItemWidgetsMap.FindChecked(ItemObject->GetItemId()))
	{
		FindWidget->RemoveFromParent();
		ItemWidgetsMap.Remove(ItemObject->GetItemId());
	}
}

void UItemsContainerGridWidget::SetupSize()
{
	FVector2D GridSize = {
		ItemsContainerRef->GetColumns() * APlayerHUD::TileSize,
		ItemsContainerRef->GetRows() * APlayerHUD::TileSize
	};
	
	GridSizeBox->SetWidthOverride(GridSize.X);
	//GridSizeBox->SetHeightOverride(GridSize.Y);
}
