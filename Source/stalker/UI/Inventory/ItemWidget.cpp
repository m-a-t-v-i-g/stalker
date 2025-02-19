﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemWidget.h"
#include "ItemDragDropOperation.h"
#include "ItemObject.h"
#include "GameHUD.h"
#include "Ammo/AmmoObject.h"
#include "Armor/ArmorObject.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Weapons/WeaponObject.h"

void UItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	MouseEnter(InGeometry, InMouseEvent);
}

void UItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	MouseLeave(InMouseEvent);
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
		return HandleLeftMouseButtonDownDoubleClick(InGeometry, InMouseEvent, EKeys::LeftMouseButton);
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

	if (auto DragVisual = CreateWidget<UItemWidget>(this, AGameHUD::StaticItemWidgetClass)) // TODO: класс виджета для драг дропа
	{
		DragVisual->SetRenderOpacity(DragOpacity);
		DragVisual->InitItemWidget(OwnerPrivate.Get(), BoundObject.Get(), {
			                           BoundObject->GetItemSize().X * 0.7, BoundObject->GetItemSize().Y * 0.7
		                           });
		DragVisual->HideAmount();
		DragDropOperation->DefaultDragVisual = DragVisual;
	}
	
	OutOperation = DragDropOperation;

	BeginDragOperation(InGeometry, InMouseEvent, OutOperation);
}

void UItemWidget::InitItemWidget(const UObject* Owner, UItemObject* BindObject, FVector2D Size)
{
	OwnerPrivate = Owner;
	BoundObject = BindObject;

	if (BoundObject.IsValid())
	{
		FSlateBrush Icon = UWidgetBlueprintLibrary::MakeBrushFromTexture(BoundObject->GetThumbnail());
		ItemImage->SetBrush(Icon);

		BoundObject->OnAmountChangeDelegate.AddUObject(this, &UItemWidget::OnChangeAmount);
		OnChangeAmount(BoundObject->GetAmount());
	}
	
	FVector2D GridSize = {Size.X * AGameHUD::TileSize, Size.Y * AGameHUD::TileSize};

	SizeBox->SetWidthOverride(GridSize.X);
	SizeBox->SetHeightOverride(GridSize.Y);
}

void UItemWidget::ClearItemWidget()
{
	OwnerPrivate.Reset();
	BoundObject.Reset();
	
	TextAmount->VisibilityDelegate.Unbind();
	TextAmount->TextDelegate.Unbind();
}

void UItemWidget::OnChangeAmount(uint32 Amount)
{
	if (!BoundObject.IsValid())
	{
		return;
	}

	BoundObject->IsCollected() ? ShowAmount() : HideAmount();

	FString AmountString = FString::Printf(TEXT("x%d"), Amount);
	if (BoundObject->IsA<UAmmoObject>())
	{
		AmountString = AmountString.RightChop(1);
	}
	
	FText AmountText = FText::FromString(AmountString);
	TextAmount->SetText(AmountText);
}

void UItemWidget::ShowAmount()
{
	if (BoundObject->IsA<UWeaponObject>() || BoundObject->IsA<UArmorObject>())
	{
		if (BoundObject->GetAmount() <= 1)
		{
			TextAmount->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}

	TextAmount->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UItemWidget::HideAmount()
{
	TextAmount->SetVisibility(ESlateVisibility::Collapsed);
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

FReply UItemWidget::HandleLeftMouseButtonDownDoubleClick(const FGeometry& InLocalGeometry,
                                                         const FPointerEvent& InMouseEvent, const FKey& DragKey)
{
	FEventReply Reply(true);
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
		DoubleClick(InLocalGeometry, InMouseEvent);
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
	if (OnMouseEnter.IsBound())
	{
		OnMouseEnter.Execute(InLocalGeometry, InMouseEvent, BoundObject.Get());
	}
}

void UItemWidget::MouseLeave(const FPointerEvent& InMouseEvent)
{
	if (OnMouseLeave.IsBound())
	{
		OnMouseLeave.Execute(InMouseEvent);
	}
}

void UItemWidget::DoubleClick(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent)
{
	if (OnDoubleClick.IsBound())
	{
		OnDoubleClick.Execute(InLocalGeometry, InMouseEvent, BoundObject.Get());
	}
}

void UItemWidget::BeginDragOperation(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                     UDragDropOperation* InOperation)
{
	RemoveFromParent();

	if (OnDragItem.IsBound())
	{
		OnDragItem.Execute(InGeometry, InMouseEvent, InOperation);
	}
}
