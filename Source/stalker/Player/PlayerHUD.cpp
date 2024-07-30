// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerHUD.h"
#include "UI/Items/InteractiveItemWidget.h"
#include "UI/Player/PlayerMainWidget.h"

UClass* APlayerHUD::StaticInteractiveItemWidgetClass {nullptr};
float APlayerHUD::TileSize {50.0f};

void APlayerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StaticInteractiveItemWidgetClass = InteractiveItemWidgetClass;
	
	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UPlayerMainWidget>(GetOwningPlayerController(), MainWidgetClass);
		if (MainWidget && !MainWidget->IsInViewport())
		{
			MainWidget->AddToViewport();
			ToggleTab(EActivateTab::HUD);
		}
	}
}

void APlayerHUD::InitializePlayerHUD(UStalkerAbilityComponent* AbilityComp, UCharacterInventoryComponent* CharInventoryComp)
{
	if (MainWidget)
	{
		MainWidget->InitializeMainWidget(AbilityComp, CharInventoryComp);
	}
}

void APlayerHUD::ToggleTab(EActivateTab Tab)
{
	if (!MainWidget) return;
	
	Tab = ActiveTab != Tab ? Tab : EActivateTab::HUD;
	MainWidget->ToggleTab(Tab);

	ActiveTab = Tab;
}

void APlayerHUD::StartLooting(UInventoryComponent* LootInventory)
{
	GetOwningPlayerController()->SetInputMode(FInputModeUIOnly());
	GetOwningPlayerController()->SetShowMouseCursor(true);
	
	ToggleTab(EActivateTab::Inventory);
	
	if (MainWidget)
	{
		MainWidget->StartLooting(LootInventory);
	}
}
