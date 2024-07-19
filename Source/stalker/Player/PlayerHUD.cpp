// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerHUD.h"
#include "UI/Items/InteractiveItemWidget.h"
#include "UI/Player/PlayerHUDWidget.h"

UClass* APlayerHUD::StaticInteractiveItemWidgetClass {nullptr};
float APlayerHUD::TileSize {50.0f};

void APlayerHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StaticInteractiveItemWidgetClass = InteractiveItemWidgetClass;
	
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UPlayerHUDWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidget && !HUDWidget->IsInViewport())
		{
			HUDWidget->InitializeHUDWidget();
			HUDWidget->AddToViewport();
		}
	}
}

void APlayerHUD::InitializePlayerInventory(UItemsContainerComponent* ItemsContainerComponent)
{
	if (HUDWidget)
	{
		HUDWidget->InitializePlayerInventory(ItemsContainerComponent);
	}
}
