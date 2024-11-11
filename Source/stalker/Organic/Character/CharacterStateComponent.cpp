// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterStateComponent.h"
#include "CharacterWeaponComponent.h"
#include "StalkerCharacter.h"
#include "StalkerCharacterMovementComponent.h"
#include "Components/OrganicAbilityComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, HealthState,		COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CombatState,		COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, MovementAction,	COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, OverlayState,	COND_SimulatedOnly);
}

void UCharacterStateComponent::SetupStateComponent()
{
	if (auto Character = GetOwner<AStalkerCharacter>())
	{
		CharacterRef = Character;
		
		if (AController* Controller = Character->GetController())
		{
			ControllerRef = Controller;
		}
	}

	if (CharacterRef)
	{
		AbilityComponentRef = CharacterRef->GetAbilitySystemComponent<UOrganicAbilityComponent>();
		MovementComponentRef = CharacterRef->GetCharacterMovement();
		WeaponComponentRef = CharacterRef->GetWeaponComponent<UCharacterWeaponComponent>();

		check(AbilityComponentRef);
		check(MovementComponentRef);
		check(WeaponComponentRef);

		WeaponComponentRef->OnFireStart.AddUObject(this, &UCharacterStateComponent::OnFireStart);
		WeaponComponentRef->OnFireStop.AddUObject(this, &UCharacterStateComponent::OnFireStop);
		WeaponComponentRef->OnAimingStart.AddUObject(this, &UCharacterStateComponent::OnAimingStart);
		WeaponComponentRef->OnAimingStop.AddUObject(this, &UCharacterStateComponent::OnAimingStop);
		WeaponComponentRef->OnOverlayChanged.AddUObject(this, &UCharacterStateComponent::OnOverlayChanged);
	}
}

void UCharacterStateComponent::SetMovementAction(ECharacterMovementAction NewAction, bool bForce)
{
	if (bForce || MovementAction != NewAction)
	{
		const ECharacterMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void UCharacterStateComponent::OnMovementActionChanged(ECharacterMovementAction PreviousAction)
{
	if (MovementAction != PreviousAction)
	{
		// Logic
	}
}

void UCharacterStateComponent::SetOverlayState(ECharacterOverlayState NewState, bool bForce)
{
	if (bForce || OverlayState != NewState)
	{
		const ECharacterOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);
	}
}

void UCharacterStateComponent::OnOverlayStateChanged(ECharacterOverlayState PreviousState)
{
	if (OverlayState != PreviousState)
	{
		// Logic
	}
}

void UCharacterStateComponent::SetHealthState(ECharacterHealthState NewState, bool bForce)
{
	if (bForce || HealthState != NewState)
	{
		const ECharacterHealthState Prev = HealthState;
		HealthState = NewState;
		OnHealthStateChanged(Prev);
	}
}

void UCharacterStateComponent::OnHealthStateChanged(ECharacterHealthState PreviousState)
{
	if (HealthState != PreviousState)
	{
		// Logic
	}
}

void UCharacterStateComponent::SetCombatState(ECharacterCombatState NewState, bool bForce)
{
	if (bForce || CombatState != NewState)
	{
		const ECharacterCombatState Prev = CombatState;
		CombatState = NewState;
		OnCombatStateChanged(Prev);
	}
}

void UCharacterStateComponent::OnCombatStateChanged(ECharacterCombatState PreviousState)
{
	if (CombatState != PreviousState)
	{
		// Logic
	}
}

void UCharacterStateComponent::OnFireStart()
{
	bFiring = true;

	if (CombatStateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(CombatStateTimer);
	}
	
	if (!bAiming)
	{
		SetCombatState(ECharacterCombatState::Tense);
	}
}

void UCharacterStateComponent::OnFireStop()
{
	bFiring = false;
	
	if (!bAiming)
	{
		SetRelaxTimer();
	}
}

void UCharacterStateComponent::OnAimingStart()
{
	if (MovementComponentRef)
	{
		if (CombatStateTimer.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatStateTimer);
		}

		SetCombatState(ECharacterCombatState::Ready);
		MovementComponentRef->SetRotationMode(ECharacterRotationMode::ControlDirection, true);
		bAiming = true;
	}
}

void UCharacterStateComponent::OnAimingStop()
{
	if (MovementComponentRef)
	{
		SetCombatState(ECharacterCombatState::Tense);
		MovementComponentRef->SetRotationMode(MovementComponentRef->GetInputRotationMode());
		bAiming = false;

		SetRelaxTimer();
	}
}

void UCharacterStateComponent::OnOverlayChanged(ECharacterOverlayState NewOverlay)
{
	SetOverlayState(NewOverlay, true);
}

void UCharacterStateComponent::SetRelaxTimer()
{
	if (!bFiring)
	{
		if (CombatStateTimer.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(CombatStateTimer);
			UKismetSystemLibrary::PrintString(this, FString("valid"), true, false);
		}

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this]
		{
			SetCombatState(ECharacterCombatState::Relaxed);
			GetWorld()->GetTimerManager().ClearTimer(CombatStateTimer);
		});

		GetWorld()->GetTimerManager().SetTimer(CombatStateTimer, TimerDelegate, 5.0f, false);
	}
}
