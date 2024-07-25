// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipmentSlotWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Inventory/EquipmentSlot.h"
#include "Interactive/Items/ItemObject.h"
#include "Player/PlayerHUD.h"
#include "UI/Items/InteractiveItemWidget.h"

void UEquipmentSlotWidget::SetupEquipmentSlot(const UCharacterInventoryComponent* CharInventoryComp)
{
	if (auto EquipmentSlot = CharInventoryComp->FindEquipmentSlotByName(SlotName))
	{
		OwnSlot = EquipmentSlot;
		OwnSlot->OnSlotEquipped.AddUObject(this, &UEquipmentSlotWidget::OnSlotEquipped);

		if (OwnSlot->IsEquipped())
		{
			OnSlotEquipped(OwnSlot->GetItemTag(), OwnSlot->GetBoundObject());
		}
	}
}

void UEquipmentSlotWidget::OnSlotEquipped(const FGameplayTag& ItemTag, UItemObject* BoundObject)
{
	if (auto ItemObject = Cast<UItemObject>(BoundObject))
	{
		if (UInteractiveItemWidget* ItemWidget = CreateWidget<UInteractiveItemWidget>(
			this, APlayerHUD::StaticInteractiveItemWidgetClass))
		{
			ItemWidget->InitItemWidget(ItemTag, ItemObject, ItemObject->GetItemSize());
			
			if (UCanvasPanelSlot* CanvasPanelSlot = SlotCanvas->AddChildToCanvas(ItemWidget))
			{
				FVector2D ItemSize = ItemObject->GetItemSize() * 50.0f;
				CanvasPanelSlot->SetSize(ItemSize);
			}
		}
	}
}
