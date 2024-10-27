// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StalkerPlayerController.h"
#include "AbilitySystem/AbilitySet.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "DataAssets/InputDataAsset.h"
#include "Input/PlayerInputConfig.h"
#include "Input/StalkerInputComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapons/CharacterWeaponComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetCharacterData();
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
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputLeftMouseName], ETriggerEvent::Triggered,
										   this,
										   &APlayerCharacter::IA_LeftMouseButton);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputRightMouseName], ETriggerEvent::Triggered,
										   this,
										   &APlayerCharacter::IA_RightMouseButton);

		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputCrouchName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Crouch);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputSprintName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Sprint);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputSlotName], ETriggerEvent::Started, this,
		                                   &APlayerCharacter::IA_Slot);
		EnhancedInputComponent->BindAction(GeneralInputData->InputMap[InputReloadName], ETriggerEvent::Started, this,
										   &APlayerCharacter::IA_Reload);

		if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
		{
			TArray<uint32> BindHandles;
			StalkerInputComp->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,
			                                     &ThisClass::Input_AbilityInputTagReleased, BindHandles);
		}
	}
}

void APlayerCharacter::SetCharacterData()
{
	if (AbilitySet)
	{
		AbilitySet->GiveToAbilitySystem(GetAbilitySystemComponent<UStalkerAbilityComponent>());
	}
}

void APlayerCharacter::IA_LeftMouseButton(const FInputActionValue& Value)
{
	auto CharWeapon = GetWeaponComponent<UCharacterWeaponComponent>();
	if (!CharWeapon)
	{
		return;
	}

	if (Value.Get<bool>())
	{
		CharWeapon->PlayBasicAction();
	}
	else
	{
		CharWeapon->StopBasicAction();
	}
}

void APlayerCharacter::IA_RightMouseButton(const FInputActionValue& Value)
{
	auto CharWeapon = GetWeaponComponent<UCharacterWeaponComponent>();
	if (!CharWeapon)
	{
		return;
	}

	if (Value.Get<bool>())
	{
		CharWeapon->PlayAlternativeAction();
	}
	else
	{
		CharWeapon->StopAlternativeAction();
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
		if (!SlotAction)
		{
			return;
		}

		auto Keys = Subsystem->QueryKeysMappedToAction(SlotAction);
		const auto PressedKey = Keys.FindByPredicate([&](const FKey& Key)
		{
			return GetController<APlayerController>()->IsInputKeyDown(Key);
		});

		if (!PressedKey)
		{
			return;
		}

		auto Mappings = InputMappingContext->GetMappings();
		for (int8 i = 0, a = 0; i < Mappings.Num(); i++)
		{
			if (Mappings[i].Action != SlotAction)
			{
				continue;
			}

			if (Mappings[i].Key.ToString() == PressedKey->ToString())
			{
				auto CharWeapon = GetWeaponComponent<UCharacterWeaponComponent>();
				if (!CharWeapon)
				{
					return;
				}

				CharWeapon->ServerToggleSlot(a);
				return;
			}
			a++;
		}
	}
}

void APlayerCharacter::IA_Reload(const FInputActionValue& Value)
{
	if (!CheckReloadAbility())
	{
		return;
	}

	auto CharWeapon = GetWeaponComponent<UCharacterWeaponComponent>();
	if (!CharWeapon)
	{
		return;
	}

	CharWeapon->TryReloadWeapon();
}

void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UStalkerAbilityComponent* LyraASC = GetAbilitySystemComponent<UStalkerAbilityComponent>())
	{
		LyraASC->AbilityInputTagPressed(InputTag);
	}
}

void APlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UStalkerAbilityComponent* LyraASC = GetAbilitySystemComponent<UStalkerAbilityComponent>())
	{
		LyraASC->AbilityInputTagReleased(InputTag);
	}
}

void APlayerCharacter::SetupCharacterLocally(AController* NewController)
{
	Super::SetupCharacterLocally(NewController);
	
	auto StalkerController = GetController<AStalkerPlayerController>();
	if (!StalkerController)
	{
		return;
	}
	
	StalkerController->OnHUDTabChanged.AddUObject(this, &APlayerCharacter::OnHUDTabChanged);
}

void APlayerCharacter::OnHUDTabChanged(EHUDTab Tab)
{
	switch (Tab)
	{
	case EHUDTab::Inventory:
		
		break;
	default:
		
		break;
	}
}
