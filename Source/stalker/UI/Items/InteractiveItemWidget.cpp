// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveItemWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Interactive/Items/ItemObject.h"
#include "Library/GeneralLibrary.h"
#include "Player/PlayerHUD.h"

void UInteractiveItemWidget::InitItemWidget(const FGameplayTag& InItemTag, const UObject* BindObject, FIntPoint Size)
{
	ItemTag = InItemTag;
	BoundObject = BindObject;

	FVector2D GridSize = {Size.X * APlayerHUD::TileSize, Size.Y * APlayerHUD::TileSize};

	SizeBox->SetWidthOverride(GridSize.X);
	SizeBox->SetHeightOverride(GridSize.Y);

	TextAmount->VisibilityDelegate.BindDynamic(this, &UInteractiveItemWidget::GetAmountVisibility);
	TextAmount->TextDelegate.BindDynamic(this, &UInteractiveItemWidget::GetAmountText);
	
	auto DescTable = DescriptionsTable.LoadSynchronous();
	if (!DescTable) return;
	
	if (auto Row = DescTable->FindRow<FTableRowDescriptions>(ItemTag.GetTagName(), ""))
	{
		auto Icon = UWidgetBlueprintLibrary::MakeBrushFromTexture(Row->Thumbnail.LoadSynchronous());
		ItemImage->SetBrush(Icon);
	}
}

ESlateVisibility UInteractiveItemWidget::GetAmountVisibility()
{
	ESlateVisibility AmountVisibility = ESlateVisibility::Collapsed;
	{
		if (auto ItemObject = GetBoundObject<UItemObject>())
		{
			if (ItemObject->IsStackable())
			{
				AmountVisibility = ESlateVisibility::SelfHitTestInvisible;
			}
		}
	}
	return AmountVisibility;
}

FText UInteractiveItemWidget::GetAmountText()
{
	FText AmountText;
	if (auto ItemObject = GetBoundObject<UItemObject>())
	{
		AmountText = FText::FromString(FString::Printf(TEXT("x%d"), ItemObject->GetItemParams().Amount));
	}
	return AmountText;
}
