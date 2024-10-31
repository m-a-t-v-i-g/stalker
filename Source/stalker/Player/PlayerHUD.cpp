// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerHUD.h"
#include "Inventory/ItemWidget.h"
#include "Player/PlayerMainWidget.h"
#include "Player/PlayerManagerWidget.h"

UClass* APlayerHUD::StaticInventoryWidgetClass {nullptr};
UClass* APlayerHUD::StaticItemWidgetClass {nullptr};
float APlayerHUD::TileSize {50.0f};

void APlayerHUD::OpenInventory()
{
	MainWidget->OpenInventory();
}

void APlayerHUD::CloseInventory()
{
	MainWidget->CloseInventory();
}

void APlayerHUD::PostInitializeComponents()
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

void APlayerHUD::InitializePlayerHUD(UOrganicAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp)
{
	if (MainWidget)
	{
		MainWidget->InitializeMainWidget(AbilityComp, CharInventoryComp);
	}
}

void APlayerHUD::ToggleTab(EHUDTab& Tab, bool bForce)
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

void APlayerHUD::StartLooting(UItemsContainerComponent* LootItemsContainer)
{
	EHUDTab DesiredTab = EHUDTab::Inventory;
	ToggleTab(DesiredTab, true);
	
	if (MainWidget)
	{
		MainWidget->StartLooting(LootItemsContainer);
	}
}
