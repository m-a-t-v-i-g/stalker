// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemWidget.h"
#include "ItemDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "InteractiveObjects/ItemSystem/ItemObject.h"
#include "StalkerHUD.h"

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

	DragDropOperation->Payload = BoundObject.Get();
	DragDropOperation->Pivot = EDragPivot::CenterCenter;

	if (auto DragVisual = CreateWidget<UItemWidget>(this, AStalkerHUD::StaticItemWidgetClass)) // TODO: класс виджета для драг дропа
	{
		DragVisual->InitItemWidget(OwnerPrivate.Get(), BoundObject.Get(),
		                           {BoundObject->GetItemSize().X, BoundObject->GetItemSize().Y});
		DragDropOperation->DefaultDragVisual = DragVisual;
	}
	
	OutOperation = DragDropOperation;

	BeginDragOperation(InGeometry, InMouseEvent, OutOperation);
}

void UItemWidget::InitItemWidget(const UObject* Owner, UItemObject* BindObject, FIntPoint Size)
{
	OwnerPrivate = Owner;
	BoundObject = BindObject;

	FVector2D GridSize = {Size.X * AStalkerHUD::TileSize, Size.Y * AStalkerHUD::TileSize};

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
	SizeBox->SetRenderTransformAngle(-90.0f);
}

void UItemWidget::UnRotateItem()
{
	SizeBox->SetRenderTransformAngle(0.0f);
}

void UItemWidget::MouseEnter(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent)
{
	OnMouseEnter.ExecuteIfBound(InLocalGeometry, InMouseEvent, BoundObject.Get());
}

void UItemWidget::MouseLeave()
{
	OnMouseLeave.ExecuteIfBound();
}

void UItemWidget::DoubleClick()
{
	OnDoubleClick.ExecuteIfBound(BoundObject.Get());
}

void UItemWidget::BeginDragOperation(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation* InOperation)
{
	RemoveFromParent();
	OnDragItem.ExecuteIfBound(InGeometry, InMouseEvent, InOperation);
}

ESlateVisibility UItemWidget::GetAmountVisibility()
{
	ESlateVisibility AmountVisibility = ESlateVisibility::Collapsed;

	if (auto ItemObject = GetBoundObject<UItemObject>())
	{
		if (ItemObject->IsCollected())
		{
			AmountVisibility = ESlateVisibility::SelfHitTestInvisible;
		}
	}
	
	return AmountVisibility;
}

FText UItemWidget::GetAmountText()
{
	FText AmountText;
	if (auto ItemObject = GetBoundObject<UItemObject>())
	{
		AmountText = FText::FromString(FString::Printf(TEXT("x%d"), ItemObject->GetAmount()));
	}
	return AmountText;
}
