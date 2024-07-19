// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerGridWidget.h"
#include "InteractiveItemWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Items/ItemsContainerComponent.h"
#include "Interactive/Items/ItemObject.h"
#include "Player/PlayerHUD.h"

void UItemsContainerGridWidget::SetupItemsContainerGrid(UItemsContainerComponent* NewItemsContainer)
{
	if (ItemsContainerComponent.IsValid())
	{
		ItemsContainerComponent->OnItemAddedToContainer.RemoveAll(this);
	}
	
	ItemsContainerComponent = NewItemsContainer;
	
	ItemsContainerComponent->OnItemAddedToContainer.AddUObject(this, &UItemsContainerGridWidget::OnItemAddedToContainer);
	ItemsContainerComponent->OnItemRemovedFromContainer.AddUObject(this, &UItemsContainerGridWidget::OnItemRemovedFromContainer);

	SetupSize();
}

void UItemsContainerGridWidget::OnItemAddedToContainer(const UItemObject* ItemObject, FIntPoint Tile)
{
	if (!IsValid(ItemObject)) return;
	
	if (UInteractiveItemWidget* ItemWidget = CreateWidget<UInteractiveItemWidget>(this, APlayerHUD::StaticInteractiveItemWidgetClass))
	{
		ItemWidget->InitItemWidget(ItemObject, ItemObject->Size);
		FVector2D WidgetPosition = {Tile.X * APlayerHUD::TileSize, Tile.Y * APlayerHUD::TileSize};

		ContainerItems.Add(ItemObject, ItemWidget);
		
		if (UCanvasPanelSlot* CanvasPanelSlot = GridCanvas->AddChildToCanvas(ItemWidget))
		{
			CanvasPanelSlot->SetAutoSize(true);
			CanvasPanelSlot->SetPosition(WidgetPosition);
		}
	}
}

void UItemsContainerGridWidget::OnItemRemovedFromContainer(const UItemObject* ItemObject)
{
	if (auto FindWidget = ContainerItems.FindChecked(ItemObject))
	{
		FindWidget->RemoveFromParent();
		ContainerItems.Remove(ItemObject);
	}
}

void UItemsContainerGridWidget::SetupSize()
{
	FVector2D GridSize = {
		ItemsContainerComponent->GetColumns() * APlayerHUD::TileSize,
		ItemsContainerComponent->GetRows() * APlayerHUD::TileSize
	};
	
	GridSizeBox->SetWidthOverride(GridSize.X);
	GridSizeBox->SetHeightOverride(GridSize.Y);
}
