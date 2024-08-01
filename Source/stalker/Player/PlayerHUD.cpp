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
			
			EHUDTab DesiredTab = EHUDTab::HUD;
			ToggleTab(DesiredTab, true);
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

void APlayerHUD::ToggleTab(EHUDTab& Tab, bool bForce)
{
	if (!MainWidget) return;
	
	Tab = ActiveTab != Tab || bForce ? Tab : EHUDTab::HUD;

	MainWidget->ToggleTab(Tab);
	ActiveTab = Tab;
	
	switch (ActiveTab)
	{
	case EHUDTab::Inventory:
	case EHUDTab::PDA:
		GetOwningPlayerController()->SetInputMode(FInputModeGameAndUI());
		GetOwningPlayerController()->SetShowMouseCursor(true);
		break;
	default:
		GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
		GetOwningPlayerController()->SetShowMouseCursor(false);
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
