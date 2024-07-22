// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMainWidget.h"
#include "PlayerInventoryWidget.h"
#include "Components/WidgetSwitcher.h"

void UPlayerMainWidget::InitializeHUDWidget()
{
}

void UPlayerMainWidget::InitializePlayerInventory(UItemsContainerComponent* ItemsContainerComponent)
{
	Inventory->InitializeInventory(ItemsContainerComponent);
}

void UPlayerMainWidget::ToggleTab(EActivateTab ActivateTab)
{
	switch (ActivateTab)
	{
	case EActivateTab::Inventory:
		TabSwitcher->SetActiveWidget(Inventory);
		break;
	case EActivateTab::PDA:
		TabSwitcher->SetActiveWidget(PDA);
		break;
	default:
		TabSwitcher->SetActiveWidget(HUD);
		break;
	}
}
