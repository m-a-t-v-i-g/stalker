// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.generated.h"

namespace FCharacterSocketName
{
	static const FName NAME_LeftHand =	"LeftHand";
	static const FName NAME_RightHand = "RightHand";
}

UENUM(BlueprintType)
enum class ECharacterMovementAction : uint8
{
	None,
	LowMantle,
	HighMantle,
	GettingUp
};

UENUM(BlueprintType)
enum class ECharacterOverlayState : uint8
{
	Default,
	OnlyLeftHand,
	OnlyRightHand,
	LeftAndRightHand,
	BothHands,
	
	Item,
	Weapon,

	// DEPRECATED
	Rifle,
	Pistol_1H,
	Pistol_2H,
	Binocular
};

UENUM(BlueprintType)
enum class ECharacterHealthState : uint8
{
	Normal	 UMETA(DisplayName = "Normal"),
	Injured	 UMETA(DisplayName = "Injured"),
	Dead	 UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECharacterCombatState : uint8
{
	Relaxed	 UMETA(DisplayName = "Relaxed"),
	Tense	 UMETA(DisplayName = "Tense"),
	Ready	 UMETA(DisplayName = "Ready")
};

UENUM(BlueprintType)
enum class ECharacterMantleType : uint8
{
	HighMantle,
	LowMantle,
	FallingCatch
};

USTRUCT(BlueprintType)
struct FCharacterMovementAction
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Health State")
	ECharacterMovementAction Action = ECharacterMovementAction::None;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
	bool bNone = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
	bool bLowMantle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
	bool bHighMantle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
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
struct FCharacterOverlayState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Overlay State")
	ECharacterOverlayState State = ECharacterOverlayState::Default;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bDefault = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bLeftHand = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bRightHand = false;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bLeftAndRightHand = false;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bBothHands = false;
	
	// DEPRECATED
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bItem = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bRifle = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bPistol_1H = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bPistol_2H = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Overlay State")
	bool bBinocular = false;

public:
	FCharacterOverlayState()
	{
	}

	FCharacterOverlayState(const ECharacterOverlayState InitialState) { *this = InitialState; }

	const bool& Default() const { return bDefault; }
	const bool& LeftHand() const { return bLeftHand; }
	const bool& RightHand() const { return bRightHand; }
	const bool& LeftAndRightHand() const { return bLeftAndRightHand; }
	const bool& BothHands() const { return bBothHands; }
	
	// DEPRECATED
	
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
		bLeftHand = State == ECharacterOverlayState::OnlyLeftHand;
		bRightHand = State == ECharacterOverlayState::OnlyRightHand;
		bLeftAndRightHand = State == ECharacterOverlayState::LeftAndRightHand;
		bBothHands = State == ECharacterOverlayState::BothHands;
		
		// DEPRECATED
		
		bItem = State == ECharacterOverlayState::Item;
		bRifle = State == ECharacterOverlayState::Rifle;
		bPistol_1H = State == ECharacterOverlayState::Pistol_1H;
		bPistol_2H = State == ECharacterOverlayState::Pistol_2H;
		bBinocular = State == ECharacterOverlayState::Binocular;
	}
};

USTRUCT(BlueprintType)
struct FCharacterHealthState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Health State")
	ECharacterHealthState State = ECharacterHealthState::Normal;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
	bool bNormal = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
	bool bInjured = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Health State")
	bool bDead = false;

public:
	FCharacterHealthState()
	{
	}

	FCharacterHealthState(const ECharacterHealthState InitialState) { *this = InitialState; }

	const bool& Normal() const { return bNormal; }
	const bool& Injured() const { return bInjured; }
	const bool& Dead() const { return bDead; }

	operator ECharacterHealthState() const { return State; }

	void operator=(const ECharacterHealthState NewState)
	{
		State = NewState;
		bNormal = State == ECharacterHealthState::Normal;
		bInjured = State == ECharacterHealthState::Injured;
		bDead = State == ECharacterHealthState::Dead;
	}
};

USTRUCT(BlueprintType)
struct FCharacterCombatState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Character|Combat State")
	ECharacterCombatState State = ECharacterCombatState::Relaxed;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Combat State")
	bool bRelaxed = true;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Combat State")
	bool bTense = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Character|Combat State")
	bool bReady = false;

public:
	FCharacterCombatState()
	{
	}

	FCharacterCombatState(const ECharacterCombatState InitialState) { *this = InitialState; }

	const bool& Relaxed() const { return bRelaxed; }
	const bool& Tense() const { return bTense; }
	const bool& Ready() const { return bReady; }

	operator ECharacterCombatState() const { return State; }

	void operator=(const ECharacterCombatState NewState)
	{
		State = NewState;
		bRelaxed = State == ECharacterCombatState::Relaxed;
		bTense = State == ECharacterCombatState::Tense;
		bReady = State == ECharacterCombatState::Ready;
	}
};
