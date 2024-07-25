// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemDraggedWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Library/GeneralLibrary.h"
#include "Player/PlayerHUD.h"

void UItemDraggedWidget::SetupDraggedWidget(const FGameplayTag& ItemTag, FIntPoint Size)
{
	FVector2D GridSize = {Size.X * APlayerHUD::TileSize, Size.Y * APlayerHUD::TileSize};

	SizeBox->SetWidthOverride(GridSize.X);
	SizeBox->SetHeightOverride(GridSize.Y);

	auto DescTable = DescriptionsTable.LoadSynchronous();
	if (!DescTable) return;
	
	if (auto Row = DescTable->FindRow<FTableRowDescriptions>(ItemTag.GetTagName(), ""))
	{
		auto Icon = UWidgetBlueprintLibrary::MakeBrushFromTexture(Row->Thumbnail.LoadSynchronous());
		ItemImage->SetBrush(Icon);
	}
}
