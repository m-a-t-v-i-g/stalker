// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StalkerPlayerController.h"
#include "DataAssets/InputDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapons/CharacterWeaponComponent.h"
#include "Weapons/WeaponComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.Get())
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetController<APlayerController>()->GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = 1;
		
		Subsystem->AddMappingContext(InputMappingContext, 1, Options);
	}
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::BindDirectionalInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputMoveName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Move);
	}
}

void APlayerCharacter::BindViewInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputViewName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_View);
	}
}

void APlayerCharacter::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputJumpName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Jump);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputCrouchName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Crouch);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputSprintName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Sprint);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputSlotName], ETriggerEvent::Started, this,
		                                   &APlayerCharacter::IA_Slot);
	}
}

void APlayerCharacter::IA_Move(const FInputActionValue& Value)
{
	Super::MoveForward(Value.Get<FVector2D>().X);
	Super::MoveRight(Value.Get<FVector2D>().Y);
}

void APlayerCharacter::IA_View(const FInputActionValue& Value)
{
	Super::TurnView(Value.Get<FVector2D>().X);
	Super::PitchView(Value.Get<FVector2D>().Y);
}

void APlayerCharacter::IA_Jump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Super::StartAction1();
	}
	else
	{
		Super::StopAction1();
	}
}

void APlayerCharacter::IA_Crouch(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Super::StartAction2();
	}
	else
	{
		Super::StopAction2();
	}
}

void APlayerCharacter::IA_Sprint(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Super::StartAction3();
	}
	else
	{
		Super::StopAction3();
	}
}

void APlayerCharacter::IA_Slot(const FInputActionValue& Value)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetController<APlayerController>()->GetLocalPlayer()))
	{
		auto SlotAction = GeneralInputData->InputMap[InputSlotName];
		if (!SlotAction) return;

		auto Keys = Subsystem->QueryKeysMappedToAction(SlotAction);
		const auto PressedKey = Keys.FindByPredicate([&](const FKey& Key)
		{
			return GetController<APlayerController>()->IsInputKeyDown(Key);
		});

		if (!PressedKey) return;

		auto Mappings = InputMappingContext->GetMappings();
		for (int8 i = 0, a = 0; i < Mappings.Num(); i++)
		{
			if (Mappings[i].Action != SlotAction) continue;

			if (Mappings[i].Key.ToString() == PressedKey->ToString())
			{
				auto CharWeapon = GetWeaponComponent<UCharacterWeaponComponent>();
				if (!CharWeapon) return;

				CharWeapon->ServerToggleSlot(a);
				return;
			}
			a++;
		}
	}
}

void APlayerCharacter::SetupCharacterLocally(AStalkerPlayerController* NewController)
{
	if (!NewController) return;

	NewController->OnHUDTabChanged.AddUObject(this, &APlayerCharacter::OnHUDTabChanged);
}

void APlayerCharacter::OnHUDTabChanged(EHUDTab Tab)
{
	switch (Tab)
	{
	case EHUDTab::Inventory:
		SetRotationMode(EOrganicRotationMode::VelocityDirection, true);
		break;
	default:
		SetRotationMode(EOrganicRotationMode::LookingDirection, true);
		break;
	}
}
