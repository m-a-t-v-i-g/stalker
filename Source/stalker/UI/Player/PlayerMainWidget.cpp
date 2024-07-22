// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMainWidget.h"
#include "PlayerManagerWidget.h"
#include "Components/WidgetSwitcher.h"
#include "HUD/HUDWidget.h"

void UPlayerMainWidget::InitializeHUD(UStalkerAbilityComponent* AbilityComponent)
{
	
}

void UPlayerMainWidget::InitializeManager(UItemsContainerComponent* ItemsContainerComponent)
{
	Manager->InitializeInventory(ItemsContainerComponent);
}

void UPlayerMainWidget::ToggleTab(EActivateTab ActivateTab)
{
	switch (ActivateTab)
	{
	case EActivateTab::Inventory:
		TabSwitcher->SetActiveWidget(Manager);
		break;
	case EActivateTab::PDA:
		TabSwitcher->SetActiveWidget(PDA);
		break;
	default:
		TabSwitcher->SetActiveWidget(HUD);
		break;
	}
}
