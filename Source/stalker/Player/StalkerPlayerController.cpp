// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerPlayerController.h"

#include "CharacterArmorComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EquipmentComponent.h"
#include "InventoryComponent.h"
#include "PawnInteractionComponent.h"
#include "PlayerCharacter.h"
#include "StalkerHUD.h"
#include "Components/OrganicAbilityComponent.h"

FName AStalkerPlayerController::InventoryManagerComponentName {"Inventory Manager Component"};

FCharacterHUDInitData::FCharacterHUDInitData(AStalkerCharacter* Char): Character(Char)
{
	Character = Char;

	if (Character)
	{
		if (UAbilitySystemComponent* AbilityComp = Character->GetAbilitySystemComponent())
		{
			AddAbilitySystemComponent(AbilityComp);
		}

		if (UInventoryComponent* InventoryComp = Character->GetInventoryComponent())
		{
			AddInventoryComponent(InventoryComp);
		}

		if (UEquipmentComponent* EquipmentComp = Character->GetEquipmentComponent())
		{
			AddEquipmentComponent(EquipmentComp);
		}

		if (UCharacterArmorComponent* ArmorComp = Character->GetArmorComponent())
		{
			AddArmorComponent(ArmorComp);
		}

		if (UPawnInteractionComponent* InteractionComp = Character->GetComponentByClass<UPawnInteractionComponent>())
		{
			AddInteractionComponent(InteractionComp);
		}
	}
}

AStalkerPlayerController::AStalkerPlayerController()
{
	InventoryManager = CreateDefaultSubobject<UInventoryManagerComponent>(InventoryManagerComponentName);
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

	GameHUD = GetHUD<AStalkerHUD>();

	if (GameHUD)
	{
		GameHUD->InitializeHUD(InventoryManager);
	}
}

void AStalkerPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (GetPawn())
	{
		if (auto PawnAbilityComp = GetPawn()->GetComponentByClass<UOrganicAbilityComponent>())
		{
			PawnAbilityComp->ProcessAbilityInput(DeltaTime);
		}
	}
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AStalkerPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OnPossessedPawnChanged.AddDynamic(this, &AStalkerPlayerController::OnPawnChanged);
}

void AStalkerPlayerController::OnPawnChanged(APawn* InOldPawn, APawn* InNewPawn)
{
	if (InNewPawn)
	{
		ChooseAndSetupPawn(InNewPawn);
	}
	else
	{
		ChooseAndClearPawn(InOldPawn);
	}
}

void AStalkerPlayerController::ChooseAndSetupPawn(APawn* InPawn)
{
	if (InPawn->IsA<AStalkerCharacter>())
	{
		if (InPawn != Stalker)
		{
			ClearCharacter();
		}

		if (bIsCharacterInitialized)
		{
			return;
		}

		SetupCharacter();
	}
}

void AStalkerPlayerController::ChooseAndClearPawn(APawn* InPawn)
{
	if (InPawn == Stalker)
	{
		ClearCharacter();
	}
}

void AStalkerPlayerController::SetupCharacter()
{
	if (!bIsCharacterInitialized)
	{
		Stalker = GetPawn<AStalkerCharacter>();

		if (Stalker)
		{
			InitEssentialComponents();
			
			if (IsLocalController())
			{
				ConnectCharacterHUD();
			}
			
			bIsCharacterInitialized = true;
		}
	}
}

void AStalkerPlayerController::ClearCharacter()
{
	if (Stalker && bIsCharacterInitialized)
	{
		UnInitEssentialComponents();

		if (IsLocalController())
		{
			DisconnectCharacterHUD();
		}
		
		Stalker = nullptr;
		bIsCharacterInitialized = false;
	}
}

void AStalkerPlayerController::ConnectCharacterHUD()
{
	if (!Stalker || !GameHUD)
	{
		return;
	}
	
	FCharacterHUDInitData UIData(Stalker);
	GameHUD->ConnectCharacterHUD(UIData);
}

void AStalkerPlayerController::DisconnectCharacterHUD()
{
	if (!Stalker || !GameHUD)
	{
		return;
	}

	GameHUD->ClearCharacterHUD();
}

void AStalkerPlayerController::InitEssentialComponents()
{
	if (InventoryManager)
	{
		InventoryManager->SetupInventoryManager(this, Stalker);
	}
}

void AStalkerPlayerController::UnInitEssentialComponents()
{
	if (InventoryManager)
	{
		InventoryManager->ResetInventoryManager();
	}
}
