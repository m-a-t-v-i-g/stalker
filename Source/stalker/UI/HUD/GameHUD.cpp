// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"

#include "EnhancedPlayerInput.h"
#include "InventoryManagerWidget.h"
#include "ItemWidget.h"
#include "MainWidget.h"
#include "PawnInteractionComponent.h"
#include "StalkerCharacter.h"
#include "StalkerGameplayTags.h"
#include "StalkerInputComponent.h"
#include "Containers/ContainerActor.h"

UClass* AGameHUD::StaticInventoryWidgetClass {nullptr};
UClass* AGameHUD::StaticItemWidgetClass {nullptr};
float AGameHUD::TileSize {75.0f};

void AGameHUD::InitializeHUD(UInventoryManagerComponent* InventoryManagerComp, UInputComponent* PlayerInputComp)
{
	check(InventoryManagerComp);
	check(PlayerInputComp);

	if (MainWidget)
	{
		MainWidget->InitializeGameWidget(InventoryManagerComp);
	}

	SetupPlayerInput(PlayerInputComp);
}

void AGameHUD::ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo)
{
	CharacterRef = HUDInitInfo.Character;
	if (CharacterRef.IsValid())
	{
		if (MainWidget)
		{
			MainWidget->ConnectCharacterPart(HUDInitInfo);
		}

		InteractionComponentRef = HUDInitInfo.InteractionComponent;
		if (InteractionComponentRef.IsValid())
		{
			InteractionComponentRef->PreInteractionDelegate.AddUObject(this, &AGameHUD::OnInteraction);
		}

		SetupCharacterInput(CharacterRef->InputComponent);
	}
}

void AGameHUD::ClearCharacterHUD()
{
	if (CharacterRef.IsValid())
	{
		if (MainWidget)
		{
			MainWidget->DisconnectCharacterPart();
		}
		
		if (InteractionComponentRef.IsValid())
		{
			InteractionComponentRef->PreInteractionDelegate.RemoveAll(this);
		}
	}

	CharacterRef.Reset();
	InteractionComponentRef.Reset();
}

void AGameHUD::OpenOwnInventory()
{
	ToggleTab(EHUDTab::Inventory, false);
}

void AGameHUD::LootOtherInventory(UInventoryComponent* OtherInventory)
{
	ToggleTab(EHUDTab::Inventory, false);
	SetupAndOpenLootingTab(OtherInventory);
}

void AGameHUD::OnInteraction(AActor* TargetActor)
{
	if (auto Container = Cast<AContainerActor>(TargetActor))
	{
		LootOtherInventory(Container->GetInventoryComponent());
	}
}

void AGameHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StaticInventoryWidgetClass = InventoryWidgetClass;
	StaticItemWidgetClass = ItemWidgetClass;

	CreateAndShowGameWidget();
}

void AGameHUD::SetupPlayerInput(UInputComponent* InputComp)
{
	check(InputComp);

	if (UEnhancedInputComponent* PlayerInput = Cast<UEnhancedInputComponent>(InputComp))
	{
		for (const FInputConfigData& InputData : InputConfigData)
		{
			if (const UInputAction* FoundAction = InputConfig->FindNativeAction(InputData.InputTag))
			{
				PlayerInput->BindAction(FoundAction, ETriggerEvent::Triggered, this, FoundAction->GetFName());
			}
		}
	}
}

void AGameHUD::SetupCharacterInput(UInputComponent* CharInputComp)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(CharInputComp))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Inventory,
										   ETriggerEvent::Triggered, this, &ThisClass::ToggleInventory);
	}
}

void AGameHUD::CreateAndShowGameWidget()
{
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

void AGameHUD::CreateAndShowMainMenuWidget()
{
	
}

void AGameHUD::ToggleTab(EHUDTab Tab, bool bForce)
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
		SetGameAndUIMode(MainWidget->GetInventoryManagerWidget(), false);
		break;
	default:
		OpenHUD();
		SetGameOnlyMode();
		break;
	}
}

void AGameHUD::OpenHUD()
{
	ClearAll();
	
	if (MainWidget)
	{
		MainWidget->OpenHUDTab();
	}
}

void AGameHUD::SetupAndOpenOwnInventory()
{
	if (MainWidget)
	{
		MainWidget->SetupAndOpenOwnInventory();
		MainWidget->ActivateSlotManager();
	}
}

void AGameHUD::CloseOwnInventory()
{
	if (MainWidget)
	{
		MainWidget->CloseOwnInventory();
	}
}

void AGameHUD::SetupAndOpenLootingTab(UInventoryComponent* InventoryToLoot)
{
	if (MainWidget)
	{
		MainWidget->OpenLootingTab(InventoryToLoot);
	}
}

void AGameHUD::ClearAll()
{
	CloseOwnInventory();
}

void AGameHUD::SetGameOnlyMode()
{
	FInputModeGameOnly GameOnlyMode;
	
	GetOwningPlayerController()->SetInputMode(GameOnlyMode);
	GetOwningPlayerController()->SetShowMouseCursor(false);
}

void AGameHUD::SetGameAndUIMode(const UWidget* WidgetToFocus, bool bShowCursorDuringCapture)
{
	FInputModeGameAndUI GameAndUIMode;
	GameAndUIMode.SetWidgetToFocus(WidgetToFocus->GetCachedWidget());
	GameAndUIMode.SetHideCursorDuringCapture(bShowCursorDuringCapture);
	
	GetOwningPlayerController()->SetInputMode(GameAndUIMode);
	GetOwningPlayerController()->SetShowMouseCursor(true);
}

void AGameHUD::SetUIOnlyMode(const UWidget* WidgetToFocus, bool bShowCursorDuringCapture)
{
	FInputModeGameAndUI UIMode;
	UIMode.SetWidgetToFocus(WidgetToFocus->GetCachedWidget());
	UIMode.SetHideCursorDuringCapture(bShowCursorDuringCapture);

	GetOwningPlayerController()->SetInputMode(UIMode);
	GetOwningPlayerController()->SetShowMouseCursor(true);
}

void AGameHUD::ToggleInventory(const FInputActionInstance& InputAction)
{
	ToggleTab(EHUDTab::Inventory, false);
}

void AGameHUD::IA_Escape(const FInputActionInstance& InputAction)
{
	if (ActiveTab != EHUDTab::HUD)
	{
		ToggleTab(EHUDTab::HUD, true);
	}
	else
	{
		CreateAndShowMainMenuWidget();
	}
}
