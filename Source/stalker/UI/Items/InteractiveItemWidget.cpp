// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveItemWidget.h"
#include "ItemDragDropOperation.h"
#include "ItemDraggedWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Interactive/Items/ItemObject.h"
#include "Player/PlayerHUD.h"

FReply UInteractiveItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return HandleLeftMouseButtonDown(InMouseEvent, EKeys::LeftMouseButton);
	}
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		return HandleRightMouseButtonDown(InMouseEvent, EKeys::RightMouseButton);
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInteractiveItemWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return HandleLeftMouseButtonDownDoubleClick(InMouseEvent, EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UInteractiveItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                  UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UItemDragDropOperation* DragDropOperation = NewObject<UItemDragDropOperation>();
	check(DragDropOperation);

	DragDropOperation->ItemWidgetRef = this;
	DragDropOperation->Payload = BoundObject.Get();
	DragDropOperation->DefaultDragVisual = this;
	DragDropOperation->Pivot = EDragPivot::CenterCenter;
	OutOperation = DragDropOperation;

	RemoveFromParent();
}

void UInteractiveItemWidget::InitItemWidget(UItemObject* BindObject, FIntPoint Size)
{
	BoundObject = BindObject;

	FVector2D GridSize = {Size.X * APlayerHUD::TileSize, Size.Y * APlayerHUD::TileSize};

	SizeBox->SetWidthOverride(GridSize.X);
	SizeBox->SetHeightOverride(GridSize.Y);

	TextAmount->VisibilityDelegate.BindDynamic(this, &UInteractiveItemWidget::GetAmountVisibility);
	TextAmount->TextDelegate.BindDynamic(this, &UInteractiveItemWidget::GetAmountText);

	auto Icon = UWidgetBlueprintLibrary::MakeBrushFromTexture(BoundObject->GetThumbnail());
	ItemImage->SetBrush(Icon);
}

FReply UInteractiveItemWidget::HandleLeftMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
		TSharedPtr<SWidget> SlateWidgetDetectingDrag = this->GetCachedWidget();
		if (SlateWidgetDetectingDrag.IsValid())
		{
			Reply.NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), DragKey);
		}
	}
	return Reply.NativeReply;
}

FReply UInteractiveItemWidget::HandleLeftMouseButtonDownDoubleClick(const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
		DoubleClick();
	}
	return Reply.NativeReply;
}

FReply UInteractiveItemWidget::HandleRightMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
	}
	return Reply.NativeReply;
}

void UInteractiveItemWidget::DoubleClick()
{
	OnDoubleClick.ExecuteIfBound(BoundObject.Get());
}

void UInteractiveItemWidget::ReverseDragOperation()
{
	OnReverseDragDropOperation.ExecuteIfBound(BoundObject.Get());
}

void UInteractiveItemWidget::CompleteDragOperation(EDragDropOperationResult OperationResult)
{
	OnCompleteDragDropOperation.ExecuteIfBound(BoundObject.Get(), OperationResult);
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
