// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/Characters/BaseCharacter.h"

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseCharacter::SetMovementAction(ECharacterMovementAction NewAction, bool bForce)
{
	if (bForce || MovementAction != NewAction)
	{
		const ECharacterMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void ABaseCharacter::OnMovementActionChanged(ECharacterMovementAction PreviousAction)
{
	
}

void ABaseCharacter::SetOverlayState(ECharacterOverlayState NewState, bool bForce)
{
	if (bForce || OverlayState != NewState)
	{
		const ECharacterOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);
	}
}

void ABaseCharacter::OnOverlayStateChanged(ECharacterOverlayState PreviousState)
{
}
