// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterStateComponent.h"
#include "CharacterWeaponComponent.h"
#include "StalkerCharacter.h"
#include "StalkerCharacterMovementComponent.h"
#include "Animation/AnimationCore.h"
#include "Attributes/HealthAttributeSet.h"
#include "Attributes/ResistanceAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/OrganicAbilityComponent.h"
#include "Net/UnrealNetwork.h"

UCharacterStateComponent::UCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bRagdoll)
	{
		UpdateRagdoll(DeltaTime);
	}
}

void UCharacterStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MovementAction);
	DOREPLIFETIME(ThisClass, OverlayState);
	DOREPLIFETIME_CONDITION(ThisClass, HealthState, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CombatState, COND_SimulatedOnly);
}

void UCharacterStateComponent::SetupStateComponent(AStalkerCharacter* InCharacter)
{
	CharacterRef = InCharacter;

	if (!CharacterRef)
	{
		UE_LOG(LogCharacter, Error,
		       TEXT("Unable to setup State Component ('%s') for character '%s': character ref is null."), *GetName(),
		       *GetOwner()->GetName());
		return;
	}
	
	MovementComponentRef = CharacterRef->GetCharacterMovement();
	AbilityComponentRef = CharacterRef->GetAbilitySystemComponent<UOrganicAbilityComponent>();
	WeaponComponentRef = CharacterRef->GetWeaponComponent<UCharacterWeaponComponent>();
}

void UCharacterStateComponent::InitCharacterInfo(AController* InController)
{
	ControllerRef = InController;
	
	if (!ControllerRef)
	{
		UE_LOG(LogCharacter, Error,
		       TEXT(
			       "Unable to setup State Component ('%s') for character '%s': controller ref is null."
		       ), *GetName(), *GetOwner()->GetName());
		return;
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

		if (const UResistanceAttributeSet* ResistanceAttribute = Cast<UResistanceAttributeSet>(
			AbilityComponentRef->GetAttributeSet(UResistanceAttributeSet::StaticClass())))
		{
			ResistanceAttributeSet = ResistanceAttribute;
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
		OnCharacterDead.Broadcast();
		StartRagdoll();
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
	switch (CombatState)
	{
	case ECharacterCombatState::Ready:
		MovementComponentRef->SetRotationMode(ECharacterRotationMode::ControlDirection, true);
		break;
	default:
		MovementComponentRef->SetRotationMode(ECharacterRotationMode::LookingDirection, true);
		break;
	}
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

void UCharacterStateComponent::StartRagdoll()
{
	OnRagdollStateChangedDelegate.Broadcast(true);

	bRagdoll = true;

	FCharacterRagdollData RagdollData;
	RagdollData.CapsuleCollisionType = GetCharacterCapsule()->GetCollisionEnabled();
	RagdollData.MeshCollisionType = GetCharacterMesh()->GetCollisionEnabled();
	RagdollData.CollisionChannel = GetCharacterMesh()->GetCollisionObjectType();
	
	PreRagdollData = RagdollData;
	
	GetCharacterCapsule()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetCharacterMesh()->SetAllBodiesBelowSimulatePhysics(FCharacterBoneName::NAME_Pelvis, true, true);
}

void UCharacterStateComponent::StopRagdoll()
{
	bRagdoll = false;
	
	GetCharacterCapsule()->SetCollisionEnabled(PreRagdollData.CapsuleCollisionType);
	GetCharacterMesh()->SetCollisionEnabled(PreRagdollData.MeshCollisionType);
	GetCharacterMesh()->SetCollisionObjectType(PreRagdollData.CollisionChannel);
	GetCharacterMesh()->SetAllBodiesSimulatePhysics(false);
	
	OnRagdollStateChangedDelegate.Broadcast(false);
}

void UCharacterStateComponent::UpdateRagdoll(float DeltaSeconds)
{
	const FVector NewRagdollVel = GetCharacterMesh()->GetPhysicsLinearVelocity(FCharacterBoneName::NAME_Root);
	LastRagdollVelocity = NewRagdollVel != FVector::ZeroVector || CharacterRef->IsLocallyControlled()
		                      ? NewRagdollVel
		                      : LastRagdollVelocity / 2;
	
	const float SpringValue = FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, 1000.0f}, {0.0f, 25000.0f}, LastRagdollVelocity.Size());
	GetCharacterMesh()->SetAllMotorsAngularDriveParams(SpringValue, 0.0f, 0.0f, false);
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

void UCharacterStateComponent::OnRep_OverlayState(ECharacterOverlayState PrevOverlayState)
{
	OnOverlayStateChanged(PrevOverlayState);
}

void UCharacterStateComponent::OnRep_HealthState(ECharacterHealthState PrevHealthState)
{
	OnHealthStateChanged(PrevHealthState);
}

void UCharacterStateComponent::OnRep_CombatState(ECharacterCombatState PrevCombatState)
{
	OnCombatStateChanged(PrevCombatState);
}

USkeletalMeshComponent* UCharacterStateComponent::GetCharacterMesh() const
{
	if (CharacterRef)
	{
		return CharacterRef->GetMesh();
	}
	return nullptr;
}

UCapsuleComponent* UCharacterStateComponent::GetCharacterCapsule() const
{
	if (CharacterRef)
	{
		return CharacterRef->GetCapsuleComponent();
	}
	return nullptr;
}
