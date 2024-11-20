// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/StalkerPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "StalkerHUD.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Components/OrganicAbilityComponent.h"

FName AStalkerPlayerController::InventoryManagerComponentName {"Inventory Manager Component"};

AStalkerPlayerController::AStalkerPlayerController()
{
	InventoryManager = CreateDefaultSubobject<UPlayerInventoryManagerComponent>(InventoryManagerComponentName);
}

void AStalkerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetupPawn();
}

void AStalkerPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	SetupPawn();
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

void AStalkerPlayerController::SetupPawn()
{
	if (!bIsPawnInitialized)
	{
		Stalker = GetPawn<APlayerCharacter>();

		if (Stalker)
		{
			InitEssentialComponents();
			
			if (IsLocalController())
			{
				ConnectHUD();
			}
			
			bIsPawnInitialized = true;
		}
	}
}

void AStalkerPlayerController::ConnectHUD()
{
	if (!Stalker || !StalkerHUD)
	{
		return;
	}

	StalkerHUD->InitializePlayerHUD(FPlayerInitInfo(
		Stalker,
		Stalker->GetAbilitySystemComponent<UOrganicAbilityComponent>(),
		Stalker->GetInventoryComponent<UCharacterInventoryComponent>(),
		Stalker->GetInteractionComponent(),
		InventoryManager));
}

void AStalkerPlayerController::InitEssentialComponents()
{
	if (InventoryManager)
	{
		InventoryManager->SetupInventoryManager(this, Stalker);
	}
}
