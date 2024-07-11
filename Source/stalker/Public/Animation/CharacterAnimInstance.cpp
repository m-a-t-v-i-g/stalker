// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CharacterAnimInstance.h"
#include "Organic/Characters/BaseCharacter.h"

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	//MovementAction = Character->GetMovementAction();
	//OverlayState = Character->GetOverlayState();
	
	//LayerBlendingValues.OverlayOverrideState = Character->GetOverlayOverrideState();
}
