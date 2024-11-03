// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerHUD.h"
#include "Inventory/ItemWidget.h"
#include "Player/PlayerMainWidget.h"
#include "Player/PlayerInventoryWidget.h"

UClass* AStalkerHUD::StaticInventoryWidgetClass {nullptr};
UClass* AStalkerHUD::StaticItemWidgetClass {nullptr};
float AStalkerHUD::TileSize {50.0f};

void AStalkerHUD::OpenInventory()
{
	MainWidget->OpenInventory();
}

void AStalkerHUD::CloseInventory()
{
	MainWidget->CloseInventory();
}

void AStalkerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StaticInventoryWidgetClass = InventoryWidgetClass;
	StaticItemWidgetClass = ItemWidgetClass;
	
	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UPlayerMainWidget>(GetOwningPlayerController(), MainWidgetClass);
		
		if (MainWidget && !MainWidget->IsInViewport())
		{
			MainWidget->AddToViewport();
			
			EHUDTab DesiredTab = EHUDTab::HUD;
			ToggleTab(DesiredTab, true);
		}
	}
}

void AStalkerHUD::InitializePlayerHUD(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp)
{
	if (MainWidget)
	{
		MainWidget->InitializeMainWidget(AbilityComp, CharInventoryComp);
	}
}

void AStalkerHUD::ToggleTab(EHUDTab& Tab, bool bForce)
{
	if (!MainWidget) return;
	
	Tab = ActiveTab != Tab || bForce ? Tab : EHUDTab::HUD;
	ActiveTab = Tab;
	
	switch (ActiveTab)
	{
	case EHUDTab::Inventory:
		GetOwningPlayerController()->SetInputMode(FInputModeGameAndUI());
		GetOwningPlayerController()->SetShowMouseCursor(true);
		OpenInventory();
		break;
	default:
		GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
		GetOwningPlayerController()->SetShowMouseCursor(false);
		CloseInventory();
		break;
	}
}

void AStalkerHUD::StartLooting(UInventoryComponent* LootItemsContainer)
{
	EHUDTab DesiredTab = EHUDTab::Inventory;
	ToggleTab(DesiredTab, true);
	
	if (MainWidget)
	{
		MainWidget->StartLooting(LootItemsContainer);
	}
}
