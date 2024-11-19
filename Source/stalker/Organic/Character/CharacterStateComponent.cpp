// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterStateComponent.h"
#include "CharacterWeaponComponent.h"
#include "StalkerCharacter.h"
#include "StalkerCharacterMovementComponent.h"
#include "Attributes/HealthAttributeSet.h"
#include "Components/OrganicAbilityComponent.h"
#include "Net/UnrealNetwork.h"

UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, HealthState,		COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CombatState,		COND_SimulatedOnly);
	DOREPLIFETIME(ThisClass, MovementAction);
	DOREPLIFETIME(ThisClass, OverlayState);
}

void UCharacterStateComponent::SetupStateComponent(AStalkerCharacter* InCharacter)
{
	CharacterRef = InCharacter;

	if (!CharacterRef)
	{
		UE_LOG(LogCharacter, Error, TEXT("Unable to setup the State Component: character ref is null."));
		return;
	}
	
	MovementComponentRef = CharacterRef->GetCharacterMovement();
	AbilityComponentRef = CharacterRef->GetAbilitySystemComponent<UOrganicAbilityComponent>();
	WeaponComponentRef = CharacterRef->GetWeaponComponent<UCharacterWeaponComponent>();
}

void UCharacterStateComponent::InitCharacterInfo(AController* InController)
{
	ControllerRef = InController;
	
	if (!CharacterRef || !ControllerRef)
	{
		UE_LOG(LogCharacter, Error,
		       TEXT(
			       "Unable to setup the State Component for character: one of the references (character or controller) is null."
		       ));
	}

	if (AbilityComponentRef)
	{
		if (const UHealthAttributeSet* HealthAttribute = Cast<UHealthAttributeSet>(
			AbilityComponentRef->GetAttributeSet(UHealthAttributeSet::StaticClass())))
		{
			HealthAttributeSet = HealthAttribute;

			FOnGameplayAttributeValueChange& MaxHealthDelegate = AbilityComponentRef->
				GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetMaxHealthAttribute());
			MaxHealthDelegate.AddUObject(this, &UCharacterStateComponent::OnMaxHealthChange);

			FOnGameplayAttributeValueChange& HealthDelegate = AbilityComponentRef->
				GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute());
			HealthDelegate.AddUObject(this, &UCharacterStateComponent::OnHealthChange);

			SetupHealth();
		}
	}

	if (WeaponComponentRef)
	{
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
	switch (HealthState)
	{
	case ECharacterHealthState::Normal:
		MovementComponentRef->SetMovementModel(NormalMovementModel);
		break;
	case ECharacterHealthState::Injured:
		MovementComponentRef->SetMovementModel(InjuredMovementModel);
		break;
	case ECharacterHealthState::Dead:
		break;
	default: break;
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
}

bool UCharacterStateComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UCharacterStateComponent::IsAutonomousProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UCharacterStateComponent::IsSimulatedProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_SimulatedProxy;
}

void UCharacterStateComponent::OnMaxHealthChange(const FOnAttributeChangeData& HealthChangeData)
{
	if (!HealthAttributeSet)
	{
		return;
	}
}

void UCharacterStateComponent::SetupHealth()
{
	if (!HealthAttributeSet)
	{
		return;
	}

	float CurrentHealth = HealthAttributeSet->GetHealth();
	if (FMath::IsNearlyZero(CurrentHealth))
	{
		SetHealthState(ECharacterHealthState::Dead, true);
	}
	else
	{
		if (CurrentHealth > NormalHealthStateLimit)
		{
			SetHealthState(ECharacterHealthState::Normal, true);
		}
		else
		{
			SetHealthState(ECharacterHealthState::Injured, true);
		}
	}
}

void UCharacterStateComponent::OnHealthChange(const FOnAttributeChangeData& HealthChangeData)
{
	SetupHealth();
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
		}

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this]
		{
			SetCombatState(ECharacterCombatState::Relaxed);
			GetWorld()->GetTimerManager().ClearTimer(CombatStateTimer);
		});

		GetWorld()->GetTimerManager().SetTimer(CombatStateTimer, TimerDelegate, CombatStateTransitionTime, false);
	}
}

void UCharacterStateComponent::OnRep_HealthState(ECharacterHealthState PrevHealthState)
{
	OnHealthStateChanged(PrevHealthState);
}

void UCharacterStateComponent::OnRep_CombatState(ECharacterCombatState PrevCombatState)
{
	OnCombatStateChanged(PrevCombatState);
}
