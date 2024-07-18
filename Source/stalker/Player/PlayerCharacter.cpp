// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DataAssets/InputDataAsset.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetController<APlayerController>()->GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = 1;
		
		Subsystem->AddMappingContext(InputMappingContext, 1, Options);
	} 
}

void APlayerCharacter::BindDirectionalInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputData->InputMap[InputMoveName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Move);
	}
}

void APlayerCharacter::BindViewInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputData->InputMap[InputViewName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_View);
	}
}

void APlayerCharacter::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(InputData->InputMap[InputJumpName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Jump);
		EnhancedInputComponent->BindAction(InputData->InputMap[InputCrouchName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Crouch);
		EnhancedInputComponent->BindAction(InputData->InputMap[InputSprintName], ETriggerEvent::Triggered, this,
		                                   &APlayerCharacter::IA_Sprint);
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
