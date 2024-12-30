// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerHUD.h"
#include "InventoryManagerWidget.h"
#include "ItemWidget.h"
#include "MainWidget.h"
#include "PlayerCharacter.h"

UClass* AStalkerHUD::StaticInventoryWidgetClass {nullptr};
UClass* AStalkerHUD::StaticItemWidgetClass {nullptr};
float AStalkerHUD::TileSize {50.0f};

void AStalkerHUD::InitializeHUD(UInventoryManagerComponent* InventoryManagerComp)
{
	if (MainWidget)
	{
		MainWidget->InitializeMainWidget(InventoryManagerComp);
	}
}

void AStalkerHUD::ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo)
{
	CharacterRef = HUDInitInfo.Character;
	if (CharacterRef.IsValid())
	{
		/* TODO */
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CharacterRef))
		{
			PlayerCharacter->OnToggleInventory.AddUObject(this, &AStalkerHUD::OnOpenInventory);
			PlayerCharacter->LootInventoryDelegate.AddUObject(this, &AStalkerHUD::OnLootInventory);
		}

		if (MainWidget)
		{
			MainWidget->ConnectCharacterPart(HUDInitInfo);
		}
	}
}

void AStalkerHUD::ClearCharacterHUD()
{
	if (CharacterRef.IsValid())
	{
		/* TODO */
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CharacterRef))
		{
			PlayerCharacter->OnToggleInventory.RemoveAll(this);
			PlayerCharacter->LootInventoryDelegate.RemoveAll(this);
		}
		
		if (MainWidget)
		{
			MainWidget->DisconnectCharacterPart();
		}
	}

	CharacterRef.Reset();
}

void AStalkerHUD::OnOpenInventory()
{
	ToggleTab(EHUDTab::Inventory, false);
}

void AStalkerHUD::OnLootInventory(UInventoryComponent* TargetInventory)
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
		MainWidget->SetupAndOpenOwnInventory();
		MainWidget->ActivateSlotManager();
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
