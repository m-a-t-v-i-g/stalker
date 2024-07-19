// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/StalkerPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter.h"
#include "PlayerHUD.h"
#include "DataAssets/InputDataAsset.h"

AStalkerPlayerController::AStalkerPlayerController()
{
}

void AStalkerPlayerController::ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass)
{
	Super::ClientSetHUD_Implementation(NewHUDClass);

	StalkerHUD = GetHUD<APlayerHUD>();
}

void AStalkerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Stalker = GetPawn<APlayerCharacter>();
	if (!Stalker) return;
	
	if (IsLocalController())
	{
		Stalker->SetupCharacterLocally();
	}
}

void AStalkerPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	Stalker = GetPawn<APlayerCharacter>();
	if (!Stalker) return;
}

void AStalkerPlayerController::SetupInputComponent()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = 1;
		
		Subsystem->AddMappingContext(InputMappingContext, 2, Options);
	}

	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(InputData->InputMap[InputInventoryName], ETriggerEvent::Triggered, this,
										   &AStalkerPlayerController::IA_Inventory);
	}
}

void AStalkerPlayerController::IA_Inventory(const FInputActionValue& Value)
{
	if (StalkerHUD)
	{
		StalkerHUD->ToggleInventory(Value.Get<bool>());
	}
}
