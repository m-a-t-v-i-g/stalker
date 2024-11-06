// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StalkerGameplayTags.h"
#include "Components/InteractionComponent.h"
#include "Components/OrganicAbilityComponent.h"
#include "Input/StalkerInputComponent.h"
#include "Player/StalkerPlayerController.h"

FName APlayerCharacter::InteractionComponentName {"Interaction Component"};

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(InteractionComponentName);
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (InteractionComponent)
	{
		InteractionComponent->SetupInteractionComponent();
	}
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
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Move, ETriggerEvent::Triggered,
										   this, &APlayerCharacter::IA_Move);
	}
}

void APlayerCharacter::BindViewInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_View, ETriggerEvent::Triggered,
										   this, &APlayerCharacter::IA_View);
	}
}

void APlayerCharacter::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		TArray<uint32> BindHandles;
		StalkerInputComp->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,
		                                     &ThisClass::Input_AbilityInputTagReleased, BindHandles);
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

void APlayerCharacter::IA_Slot(const FInputActionValue& Value)
{
	
	/*
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
	*/
	
}

void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UOrganicAbilityComponent* LyraASC = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		LyraASC->AbilityInputTagPressed(InputTag);
	}
}

void APlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UOrganicAbilityComponent* LyraASC = GetAbilitySystemComponent<UOrganicAbilityComponent>())
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
