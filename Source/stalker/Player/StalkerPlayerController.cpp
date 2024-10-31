// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/StalkerPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerHUD.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Data/InputDataAsset.h"

AStalkerPlayerController::AStalkerPlayerController()
{
}

void AStalkerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Stalker = GetPawn<APlayerCharacter>();
	if (!Stalker) return;
	
	if (IsLocalController() && StalkerHUD)
	{
		if (!bIsControllerInitialized)
		{
			StalkerHUD->InitializePlayerHUD(Stalker->GetAbilitySystemComponent<UOrganicAbilityComponent>(),
											Stalker->GetInventoryComponent<UCharacterInventoryComponent>());
			bIsControllerInitialized = true;
		}
	}
}

void AStalkerPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	Stalker = GetPawn<APlayerCharacter>();
	if (!Stalker) return;

	if (StalkerHUD)
	{
		if (!bIsControllerInitialized)
		{
			StalkerHUD->InitializePlayerHUD(Stalker->GetAbilitySystemComponent<UOrganicAbilityComponent>(),
			Stalker->GetInventoryComponent<UCharacterInventoryComponent>());
			bIsControllerInitialized = true;
		}
	}
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
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputInventoryName], ETriggerEvent::Triggered, this,
										   &AStalkerPlayerController::IA_Inventory);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputPDAName], ETriggerEvent::Triggered, this,
										   &AStalkerPlayerController::IA_PDA);
	}
}

void AStalkerPlayerController::ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass)
{
	Super::ClientSetHUD_Implementation(NewHUDClass);

	StalkerHUD = GetHUD<APlayerHUD>();
}

void AStalkerPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (auto PawnAbilityComp = GetPawn()->GetComponentByClass<UOrganicAbilityComponent>())
	{
		PawnAbilityComp->ProcessAbilityInput(DeltaTime);
	}
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AStalkerPlayerController::IA_Inventory(const FInputActionValue& Value)
{
	ToggleHUD(EHUDTab::Inventory, false);
}

void AStalkerPlayerController::IA_PDA(const FInputActionValue& Value)
{
	
}

void AStalkerPlayerController::ToggleHUD(EHUDTab Tab, bool bForce) const
{
	if (StalkerHUD)
	{
		EHUDTab DesiredTab = Tab;
		StalkerHUD->ToggleTab(DesiredTab, bForce);
		OnHUDTabChanged.Broadcast(DesiredTab);
	}
}
