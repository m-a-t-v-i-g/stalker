// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/StalkerPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "StalkerHUD.h"
#include "AbilitySystem/Components/OrganicAbilityComponent.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/PlayerCharacter.h"

AStalkerPlayerController::AStalkerPlayerController()
{
}

void AStalkerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!bIsControllerInitialized)
	{
		Stalker = GetPawn<APlayerCharacter>();
		ConnectHUD();
		bIsControllerInitialized = true;
	}
}

void AStalkerPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (!bIsControllerInitialized)
	{
		Stalker = GetPawn<APlayerCharacter>();
		ConnectHUD();
		bIsControllerInitialized = true;
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
}

void AStalkerPlayerController::ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass)
{
	Super::ClientSetHUD_Implementation(NewHUDClass);

	StalkerHUD = GetHUD<AStalkerHUD>();
}

void AStalkerPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (auto PawnAbilityComp = GetPawn()->GetComponentByClass<UOrganicAbilityComponent>())
	{
		PawnAbilityComp->ProcessAbilityInput(DeltaTime);
	}
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AStalkerPlayerController::ConnectHUD()
{
	if (IsLocalController())
	{
		if (!Stalker || !StalkerHUD)
		{
			return;
		}

		StalkerHUD->InitializePlayerHUD(FCharacterInitInfo(
			Stalker,
			Stalker->GetAbilitySystemComponent<UOrganicAbilityComponent>(),
			Stalker->GetInventoryComponent<UCharacterInventoryComponent>(),
			Stalker->GetInteractionComponent()));
	}
}
