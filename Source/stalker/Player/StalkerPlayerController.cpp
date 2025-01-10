// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerPlayerController.h"
#include "CharacterArmorComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EquipmentComponent.h"
#include "GameHUD.h"
#include "InventoryComponent.h"
#include "PawnInteractionComponent.h"
#include "StalkerCharacter.h"
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

void AStalkerPlayerController::ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass)
{
	Super::ClientSetHUD_Implementation(NewHUDClass);

	GameHUD = GetHUD<AGameHUD>();

	if (GameHUD)
	{
		GameHUD->InitializeHUD(InventoryManager, InputComponent);
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

void AStalkerPlayerController::SetupInputComponent()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = 1;
		
		Subsystem->AddMappingContext(CharacterMappingContext, 1, Options);
		Subsystem->AddMappingContext(UIMappingContext, 0, Options);
	}
	
	Super::SetupInputComponent();
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
		if (InPawn != Character)
		{
			ClearCharacter();
		}

		SetupCharacter();
	}

	InitEssentialComponents();
}

void AStalkerPlayerController::ChooseAndClearPawn(APawn* InPawn)
{
	if (InPawn == Character)
	{
		ClearCharacter();
	}
	
	UnInitEssentialComponents();
}

void AStalkerPlayerController::SetupCharacter()
{
	if (!bIsCharacterInitialized)
	{
		Character = GetPawn<AStalkerCharacter>();

		if (Character)
		{
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
	if (Character && bIsCharacterInitialized)
	{
		if (IsLocalController())
		{
			DisconnectCharacterHUD();
		}
		
		Character = nullptr;
		bIsCharacterInitialized = false;
	}
}

void AStalkerPlayerController::ConnectCharacterHUD()
{
	if (!Character || !GameHUD)
	{
		return;
	}
	
	FCharacterHUDInitData HUDData(Character);
	GameHUD->ConnectCharacterHUD(HUDData);
}

void AStalkerPlayerController::DisconnectCharacterHUD()
{
	if (!Character || !GameHUD)
	{
		return;
	}

	GameHUD->ClearCharacterHUD();
}

void AStalkerPlayerController::InitEssentialComponents()
{
	if (InventoryManager)
	{
		InventoryManager->SetupInventoryManager(GetPawn(), this);
	}
}

void AStalkerPlayerController::UnInitEssentialComponents()
{
	if (InventoryManager)
	{
		InventoryManager->ResetInventoryManager();
	}
}
