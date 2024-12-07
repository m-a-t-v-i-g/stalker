// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerHUD.h"
#include "Character/PlayerCharacter.h"
#include "HUD/MainWidget.h"
#include "Inventory/InventoryManagerWidget.h"
#include "Inventory/ItemWidget.h"

UClass* AStalkerHUD::StaticInventoryWidgetClass {nullptr};
UClass* AStalkerHUD::StaticItemWidgetClass {nullptr};
float AStalkerHUD::TileSize {50.0f};

void AStalkerHUD::InitializePlayerHUD(const FPlayerInitInfo& PlayerInitInfo)
{
	if (APlayerCharacter* PlayerCharacter = PlayerInitInfo.Character)
	{
		PlayerCharacter->OnToggleInventory.AddUObject(this, &AStalkerHUD::OnOpenInventory);
		PlayerCharacter->OnLootContainer.AddUObject(this, &AStalkerHUD::OnContainerInteract);
	}

	if (MainWidget)
	{
		MainWidget->InitializeMainWidget(PlayerInitInfo);
	}
}

void AStalkerHUD::OnOpenInventory()
{
	ToggleTab(EHUDTab::Inventory, false);
}

void AStalkerHUD::OnContainerInteract(UInventoryComponent* TargetInventory)
{
	ToggleTab(EHUDTab::Inventory, false);
	StartLooting(TargetInventory);
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
		SetupAndOpenOwnInventory();
		SetGameAndUIMode();
		break;
	default:
		OpenHUD();
		SetGameOnlyMode();
		break;
	}
}

void AStalkerHUD::SetupAndOpenOwnInventory()
{
	if (MainWidget)
	{
		MainWidget->SetupOwnInventory();
		MainWidget->OpenInventoryTab();
	}
}

void AStalkerHUD::CloseOwnInventory()
{
	if (MainWidget)
	{
		MainWidget->CloseOwnInventory();
	}
}

void AStalkerHUD::StartLooting(UInventoryComponent* InventoryToLoot)
{
	if (MainWidget)
	{
		MainWidget->OpenLootingTab(InventoryToLoot);
	}
}

void AStalkerHUD::OpenHUD()
{
	ClearAll();
	
	if (MainWidget)
	{
		MainWidget->OpenHUDTab();
	}
}

void AStalkerHUD::ClearAll()
{
	CloseOwnInventory();
}

void AStalkerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StaticInventoryWidgetClass = InventoryWidgetClass;
	StaticItemWidgetClass = ItemWidgetClass;
	
	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UMainWidget>(GetOwningPlayerController(), MainWidgetClass);
		
		if (MainWidget && !MainWidget->IsInViewport())
		{
			MainWidget->AddToViewport();
			ToggleTab(EHUDTab::HUD, true);
		}
	}
}

void AStalkerHUD::SetGameOnlyMode()
{
	GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
	GetOwningPlayerController()->SetShowMouseCursor(false);
}

void AStalkerHUD::SetGameAndUIMode()
{
	GetOwningPlayerController()->SetInputMode(FInputModeGameAndUI());
	GetOwningPlayerController()->SetShowMouseCursor(true);
}
