// Fill out your copyright notice in the Description page of Project Settings.

#include "GameHUD.h"
#include "EnhancedPlayerInput.h"
#include "InventoryManagerWidget.h"
#include "ItemWidget.h"
#include "GameWidget.h"
#include "PawnInteractionComponent.h"
#include "StalkerCharacter.h"
#include "StalkerInputComponent.h"
#include "Containers/ContainerActor.h"

UClass* AGameHUD::StaticInventoryWidgetClass {nullptr};
UClass* AGameHUD::StaticItemWidgetClass {nullptr};
float AGameHUD::TileSize {75.0f};

void AGameHUD::InitializeHUD(UInventoryManagerComponent* InventoryManagerComp, UInputComponent* PlayerInputComp)
{
	check(InventoryManagerComp);
	check(PlayerInputComp);

	if (GameWidgetRef)
	{
		GameWidgetRef->InitializeGameWidget(InventoryManagerComp);
	}

	SetupPlayerInput(PlayerInputComp);
}

void AGameHUD::ConnectCharacterHUD(const FCharacterHUDInitData& HUDInitInfo)
{
	CharacterRef = HUDInitInfo.Character;
	if (CharacterRef.IsValid())
	{
		if (GameWidgetRef)
		{
			GameWidgetRef->ConnectCharacterPart(HUDInitInfo);
		}

		InteractionComponentRef = HUDInitInfo.InteractionComponent;
		if (InteractionComponentRef.IsValid())
		{
			InteractionComponentRef->PreInteractionDelegate.AddUObject(this, &AGameHUD::OnInteraction);
		}

		CharacterRef->ToggleInventoryDelegate.AddUObject(this, &AGameHUD::ToggleInventory);
		CharacterRef->ToggleSlotDelegate.AddUObject(this, &AGameHUD::ToggleSlot);
	}
}

void AGameHUD::ClearCharacterHUD()
{
	if (CharacterRef.IsValid())
	{
		if (GameWidgetRef)
		{
			GameWidgetRef->DisconnectCharacterPart();
		}
		
		if (InteractionComponentRef.IsValid())
		{
			InteractionComponentRef->PreInteractionDelegate.RemoveAll(this);
		}
		
		CharacterRef->ToggleInventoryDelegate.RemoveAll(this);
		CharacterRef->ToggleSlotDelegate.RemoveAll(this);
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

void AGameHUD::CreateAndShowGameWidget()
{
	if (GameWidgetClass)
	{
		GameWidgetRef = CreateWidget<UGameWidget>(GetOwningPlayerController(), GameWidgetClass);
		
		if (GameWidgetRef && !GameWidgetRef->IsInViewport())
		{
			GameWidgetRef->AddToViewport();
			ToggleTab(EHUDTab::HUD, true);
		}
	}
}

void AGameHUD::CreateAndShowMainMenuWidget()
{
	
}

void AGameHUD::ToggleTab(EHUDTab Tab, bool bForce)
{
	if (!GameWidgetRef)
	{
		return;
	}

	Tab = ActiveTab != Tab || bForce ? Tab : EHUDTab::HUD;
	ActiveTab = Tab;
	
	switch (ActiveTab)
	{
	case EHUDTab::Inventory:
		SetupAndOpenOwnInventory();
		SetGameAndUIMode(GameWidgetRef->GetInventoryManagerWidget(), false);
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
	
	if (GameWidgetRef)
	{
		GameWidgetRef->OpenHUDTab();
	}
}

void AGameHUD::SetupAndOpenOwnInventory()
{
	if (GameWidgetRef)
	{
		GameWidgetRef->SetupAndOpenOwnInventory();
		GameWidgetRef->ActivateSlotManager();
	}
}

void AGameHUD::CloseOwnInventory()
{
	if (GameWidgetRef)
	{
		GameWidgetRef->CloseOwnInventory();
	}
}

void AGameHUD::SetupAndOpenLootingTab(UInventoryComponent* InventoryToLoot)
{
	if (GameWidgetRef)
	{
		GameWidgetRef->OpenLootingTab(InventoryToLoot);
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

void AGameHUD::SetGameAndUIMode(const UWidget* WidgetToFocus, bool bHideCursorDuringCapture)
{
	FInputModeGameAndUI GameAndUIMode;
	GameAndUIMode.SetWidgetToFocus(WidgetToFocus->GetCachedWidget());
	GameAndUIMode.SetHideCursorDuringCapture(bHideCursorDuringCapture);
	
	GetOwningPlayerController()->SetInputMode(GameAndUIMode);
	GetOwningPlayerController()->SetShowMouseCursor(true);
}

void AGameHUD::SetUIOnlyMode(const UWidget* WidgetToFocus, bool bHideCursorDuringCapture)
{
	FInputModeGameAndUI UIMode;
	UIMode.SetWidgetToFocus(WidgetToFocus->GetCachedWidget());
	UIMode.SetHideCursorDuringCapture(bHideCursorDuringCapture);

	GetOwningPlayerController()->SetInputMode(UIMode);
	GetOwningPlayerController()->SetShowMouseCursor(true);
}

void AGameHUD::ToggleInventory()
{
	ToggleTab(EHUDTab::Inventory, false);
}

void AGameHUD::ToggleSlot(uint8 SlotIndex)
{
	
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
