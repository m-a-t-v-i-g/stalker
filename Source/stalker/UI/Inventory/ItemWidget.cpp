// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemWidget.h"
#include "ItemDragDropOperation.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "InteractiveObjects/ItemSystem/ItemObject.h"
#include "Player/PlayerHUD.h"

void UItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	MouseEnter(InGeometry, InMouseEvent);
}

void UItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	MouseLeave();
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

FReply UItemWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return HandleLeftMouseButtonDownDoubleClick(InMouseEvent, EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                  UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UnRotateItem();
	
	UItemDragDropOperation* DragDropOperation = NewObject<UItemDragDropOperation>();
	check(DragDropOperation);

	DragDropOperation->ItemWidgetRef = this;
	DragDropOperation->Payload = BoundObject.Get();
	DragDropOperation->Pivot = EDragPivot::CenterCenter;
	auto DragVisual = CreateWidget<UItemWidget>(this, APlayerHUD::StaticInteractiveItemWidgetClass);
	if (DragVisual)
	{
		DragVisual->InitItemWidget(BoundObject.Get(), {1, 1});
	}
	DragDropOperation->DefaultDragVisual = DragVisual;
	OutOperation = DragDropOperation;

	BeginDragOperation();
}

void UItemWidget::InitItemWidget(UItemObject* BindObject, FIntPoint Size)
{
	BoundObject = BindObject;

	FVector2D GridSize = {Size.X * APlayerHUD::TileSize, Size.Y * APlayerHUD::TileSize};

	SizeBox->SetWidthOverride(GridSize.X);
	SizeBox->SetHeightOverride(GridSize.Y);

	TextAmount->VisibilityDelegate.BindDynamic(this, &UItemWidget::GetAmountVisibility);
	TextAmount->TextDelegate.BindDynamic(this, &UItemWidget::GetAmountText);

	auto Icon = UWidgetBlueprintLibrary::MakeBrushFromTexture(BoundObject->GetThumbnail());
	ItemImage->SetBrush(Icon);
}

FReply UItemWidget::HandleLeftMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
		TSharedPtr<SWidget> SlateWidgetDetectingDrag = TakeWidget();
		if (SlateWidgetDetectingDrag.IsValid())
		{
			Reply.NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), DragKey);
		}
	}
	return Reply.NativeReply;
}

FReply UItemWidget::HandleLeftMouseButtonDownDoubleClick(const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
		DoubleClick();
	}
	return Reply.NativeReply;
}

FReply UItemWidget::HandleRightMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
	}
	return Reply.NativeReply;
}

void UItemWidget::RotateItem()
{
	ItemImage->SetRenderTransformAngle(-90.0f);
}

void UItemWidget::UnRotateItem()
{
	ItemImage->SetRenderTransformAngle(0.0f);
}

void UItemWidget::MouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnMouseEnter.ExecuteIfBound(InGeometry, InMouseEvent, BoundObject.Get());
}

void UItemWidget::MouseLeave()
{
	OnMouseLeave.ExecuteIfBound(BoundObject.Get());
}

void UItemWidget::DoubleClick()
{
	OnDoubleClick.ExecuteIfBound(BoundObject.Get());
}

void UItemWidget::BeginDragOperation()
{
	RemoveFromParent();
	OnBeginDragDropOperation.ExecuteIfBound(BoundObject.Get());
}

void UItemWidget::ReverseDragOperation()
{
	OnReverseDragDropOperation.ExecuteIfBound(BoundObject.Get());
}

void UItemWidget::CompleteDragOperation(EDragDropOperationResult OperationResult)
{
	OnCompleteDragDropOperation.ExecuteIfBound(BoundObject.Get(), OperationResult);
}

ESlateVisibility UItemWidget::GetAmountVisibility()
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

FText UItemWidget::GetAmountText()
{
	FText AmountText;
	if (auto ItemObject = GetBoundObject<UItemObject>())
	{
		AmountText = FText::FromString(FString::Printf(TEXT("x%d"), ItemObject->GetItemParams().Amount));
	}
	return AmountText;
}
