// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.generated.h"

UENUM(BlueprintType)
enum class ECharacterOverlayState : uint8
{
	Default,
	Item,
	Weapon,

	// DEPRECATED
	Rifle,
	Pistol_1H,
	Pistol_2H,
	Binocular
};

UENUM(BlueprintType)
enum class ECharacterMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	GettingUp
};

UENUM(BlueprintType)
enum class ECharacterMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

USTRUCT(BlueprintType)
struct FCharacterOverlayState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	ECharacterOverlayState State = ECharacterOverlayState::Default;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	bool bDefault = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	bool bItem = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	bool bRifle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	bool bPistol_1H = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	bool bPistol_2H = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Character State")
	bool bBinocular = false;

public:
	FCharacterOverlayState()
	{
	}

	FCharacterOverlayState(const ECharacterOverlayState InitialState) { *this = InitialState; }

	const bool& Default() const { return bDefault; }
	const bool& Item() const { return bItem; }
	const bool& Rifle() const { return bRifle; }
	const bool& Pistol_1H() const { return bPistol_1H; }
	const bool& Pistol_2H() const { return bPistol_2H; }
	const bool& Binocular() const { return bBinocular; }
	
	operator ECharacterOverlayState() const { return State; }

	void operator=(const ECharacterOverlayState NewAction)
	{
		State = NewAction;
		bDefault = State == ECharacterOverlayState::Default;
		bItem = State == ECharacterOverlayState::Item;
		bRifle = State == ECharacterOverlayState::Rifle;
		bPistol_1H = State == ECharacterOverlayState::Pistol_1H;
		bPistol_2H = State == ECharacterOverlayState::Pistol_2H;
		bBinocular = State == ECharacterOverlayState::Binocular;
	}
};

USTRUCT(BlueprintType)
struct FCharacterMovementAction
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement System")
	ECharacterMovementAction Action = ECharacterMovementAction::None;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement System")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement System")
	bool bLowMantle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement System")
	bool bHighMantle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Movement System")
	bool bGettingUp = false;

public:
	FCharacterMovementAction()
	{
	}

	FCharacterMovementAction(const ECharacterMovementAction InitialAction) { *this = InitialAction; }

	const bool& None() const { return bNone; }
	const bool& LowMantle() const { return bLowMantle; }
	const bool& HighMantle() const { return bHighMantle; }
	const bool& GettingUp() const { return bGettingUp; }

	operator ECharacterMovementAction() const { return Action; }

	void operator=(const ECharacterMovementAction NewAction)
	{
		Action = NewAction;
		bNone = Action == ECharacterMovementAction::None;
		bLowMantle = Action == ECharacterMovementAction::LowMantle;
		bHighMantle = Action == ECharacterMovementAction::HighMantle;
		bGettingUp = Action == ECharacterMovementAction::GettingUp;
	}
};

USTRUCT(BlueprintType)
struct FTableRowCharacterOverlays : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	ECharacterOverlayState OverlayState;
};
