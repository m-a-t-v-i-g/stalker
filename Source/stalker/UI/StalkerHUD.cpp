// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerHUD.h"

#include "Character/PlayerCharacter.h"
#include "Inventory/ItemWidget.h"
#include "Player/PlayerMainWidget.h"
#include "Player/PlayerInventoryWidget.h"

UClass* AStalkerHUD::StaticInventoryWidgetClass {nullptr};
UClass* AStalkerHUD::StaticItemWidgetClass {nullptr};
float AStalkerHUD::TileSize {50.0f};

void AStalkerHUD::InitializePlayerHUD(const FCharacterInitInfo& CharacterInitInfo)
{
	if (APlayerCharacter* PlayerCharacter = CharacterInitInfo.Character)
	{
		//PlayerCharacter->ToggleHUDTab.AddUObject(this, &AStalkerHUD::ToggleTab);
	}
	
	if (MainWidget)
	{
		MainWidget->InitializeMainWidget(CharacterInitInfo);
	}
}

void AStalkerHUD::ToggleTab(EHUDTab Tab, bool bForce)
{
	if (!MainWidget)
	{
		return;
	}
	
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

void AStalkerHUD::OpenInventory()
{
	if (MainWidget)
	{
		MainWidget->OpenInventory();
	}
}

void AStalkerHUD::CloseInventory()
{
	if (MainWidget)
	{
		MainWidget->CloseInventory();
	}
}

void AStalkerHUD::StartLooting(UInventoryComponent* InventoryToLoot)
{
	ToggleTab(EHUDTab::Inventory, true);
	
	if (MainWidget)
	{
		MainWidget->OpenInventory();
		MainWidget->StartLooting(InventoryToLoot);
	}
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
			ToggleTab(EHUDTab::HUD, true);
		}
	}
}
