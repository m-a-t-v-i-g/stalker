// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Library/CharacterLibrary.h"
#include "Organic/BaseOrganic.h"
#include "BaseCharacter.generated.h"

UCLASS()
class STALKER_API ABaseCharacter : public ABaseOrganic
{
	GENERATED_BODY()

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

#pragma region Movement

protected:
	UPROPERTY(VisibleInstanceOnly, Category = "Character|Movement")
	ECharacterMovementAction MovementAction = ECharacterMovementAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|State")
	ECharacterOverlayState OverlayState = ECharacterOverlayState::Default;

public:
	void SetMovementAction(ECharacterMovementAction NewAction, bool bForce = false);
	FORCEINLINE ECharacterMovementAction GetMovementAction() const { return MovementAction; }
	
	virtual void OnMovementActionChanged(ECharacterMovementAction PreviousAction);

#pragma endregion Movement
	
#pragma region Overlay State

	void SetOverlayState(ECharacterOverlayState NewState, bool bForce = false);
	FORCEINLINE ECharacterOverlayState GetOverlayState() const { return OverlayState; }

	virtual void OnOverlayStateChanged(ECharacterOverlayState PreviousState);

#pragma endregion Overlay State
};
