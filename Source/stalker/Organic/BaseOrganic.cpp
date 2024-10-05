// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/BaseOrganic.h"
#include "GenCapsuleComponent.h"
#include "AbilitySystem/Attributes/OrganicAttributeSet.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"
#include "Components/Weapons/WeaponComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

FName ABaseOrganic::MeshName {"OrganicMesh0"};
FName ABaseOrganic::OrganicMovementName {"OrganicMoveComp"};
FName ABaseOrganic::CapsuleName {"OrganicCollision"};
FName ABaseOrganic::AbilitySystemComponentName {"AbilityComp"};
FName ABaseOrganic::InventoryComponentName {"InventoryComp"};
FName ABaseOrganic::WeaponComponentName {"WeaponComp"};

const FName NAME_RotationAmount("RotationAmount");
const FName NAME_YawOffset("YawOffset");

ABaseOrganic::ABaseOrganic(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.Get())
{
	struct FConstructorStatics
	{
		FName ID_Characters;
		FText NAME_Characters;
		FConstructorStatics()
			: ID_Characters(TEXT("Characters"))
			, NAME_Characters(NSLOCTEXT("SpriteCategory", "Characters", "Characters"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	CapsuleComponent = CreateDefaultSubobject<UGenCapsuleComponent>(CapsuleName);
	if (CapsuleComponent)
	{
		CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
		CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

		CapsuleComponent->CanCharacterStepUpOn = ECB_No;
		CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
		CapsuleComponent->SetCanEverAffectNavigation(false);
		CapsuleComponent->bDynamicObstacle = true;
		SetRootComponent(CapsuleComponent);
	}
	
	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(MeshName);
	if (Mesh)
	{
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetupAttachment(CapsuleComponent);
		Mesh->SetCollisionProfileName("CharacterMesh");
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}
	
	OrganicMovement = CreateDefaultSubobject<UOrganicMovementComponent>(OrganicMovementName);
	if (OrganicMovement)
	{
		OrganicMovement->UpdatedComponent = CapsuleComponent;
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UStalkerAbilityComponent>(AbilitySystemComponentName);
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	OrganicAttributeSet = CreateDefaultSubobject<UOrganicAttributeSet>("OrganicAttributeSet");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(WeaponComponentName);
	
#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>("Arrow");
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = ConstructorStatics.ID_Characters;
		ArrowComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Characters;
		ArrowComponent->SetupAttachment(CapsuleComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif
	
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	SetReplicatingMovement(false);
}

void ABaseOrganic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetMesh())
	{
		if (GetOrganicMovement() && GetMesh()->PrimaryComponentTick.bCanEverTick)
		{
			GetMesh()->PrimaryComponentTick.AddPrerequisite(GetOrganicMovement(), GetOrganicMovement()->PrimaryComponentTick);
		}
	}
	
	if (GetOrganicMovement() && GetCapsuleComponent())
	{
		GetOrganicMovement()->UpdateNavAgent(*CapsuleComponent);
	}
}

void ABaseOrganic::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh())
	{
		GetMesh()->AddTickPrerequisiteActor(this);
	}
	
	SetMovementModel();
	ForceUpdateCharacterState();

	TargetRotation = GetOrganicMovement()->GetRootCollisionRotation();
	LastVelocityRotation = TargetRotation;
	
	GetOrganicMovement()->SetMovementSettings(GetMovementSettings());
}

void ABaseOrganic::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (GetAbilitySystemComponent<UStalkerAbilityComponent>())
	{
		GetAbilitySystemComponent<UStalkerAbilityComponent>()->InitAbilitySystem(NewController, this);
	}
}

void ABaseOrganic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseOrganic, RotationMode, COND_SimulatedOnly);
}

UAbilitySystemComponent* ABaseOrganic::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void ABaseOrganic::OnMovementModeChanged()
{
	if (GetOrganicMovement()->IsMovingOnGround())
	{
		SetMovementState(EOrganicMovementState::Ground);
	}
	else if (GetOrganicMovement()->IsAirborne())
	{
		SetMovementState(EOrganicMovementState::Airborne);
	}
}

void ABaseOrganic::SetMovementModel()
{
	const FString ContextString = GetFullName();
	const FOrganicMovementModel* OutRow = MovementTable.DataTable->FindRow<FOrganicMovementModel>(MovementTable.RowName, ContextString);
	if (!OutRow) return;
	
	MovementModel = *OutRow;
}

FOrganicMovementSettings ABaseOrganic::GetMovementSettings() const
{
	switch (RotationMode)
	{
	case EOrganicRotationMode::ControlDirection:
		{
			if (Stance == EOrganicStance::Standing)
			{
				return MovementModel.ControlDirection.Standing;
			}
			if (Stance == EOrganicStance::Crouching)
			{
				return MovementModel.ControlDirection.Crouching;
			}
		}
	case EOrganicRotationMode::LookingDirection:
		{
			if (Stance == EOrganicStance::Standing)
			{
				return MovementModel.LookingDirection.Standing;
			}
			if (Stance == EOrganicStance::Crouching)
			{
				return MovementModel.LookingDirection.Crouching;
			}
		}
	case EOrganicRotationMode::VelocityDirection:
		{
			if (Stance == EOrganicStance::Standing)
			{
				return MovementModel.VelocityDirection.Standing;
			}
			if (Stance == EOrganicStance::Crouching)
			{
				return MovementModel.VelocityDirection.Crouching;
			}
		}
	default: return MovementModel.VelocityDirection.Standing;
	}
}

void ABaseOrganic::SetMovementState(const EOrganicMovementState NewMovementState, bool bForce)
{
	if (bForce || MovementState != NewMovementState)
	{
		PrevMovementState = MovementState;
		MovementState = NewMovementState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void ABaseOrganic::SetStance(const EOrganicStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const EOrganicStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void ABaseOrganic::OnStanceChanged(EOrganicStance PreviousStance)
{
	OrganicMovement->SetMovementSettings(GetMovementSettings());
}

void ABaseOrganic::GaitTick()
{
	const EOrganicGait AllowedGait = CalculateAllowedGait();
	const EOrganicGait ActualGait = CalculateActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}
	OrganicMovement->SetAllowedGait(AllowedGait);
}

void ABaseOrganic::SetInputGait(EOrganicGait NewInputGait)
{
	InputGait = NewInputGait;
}

void ABaseOrganic::SetGait(const EOrganicGait NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		const EOrganicGait PreviousGait = Gait;
		Gait = NewGait;
		OnGaitChanged(PreviousGait);
	}
}

void ABaseOrganic::OnGaitChanged(EOrganicGait PreviousGait)
{
	
}

EOrganicGait ABaseOrganic::CalculateAllowedGait() const
{
	if (Stance == EOrganicStance::Standing)
	{
		if (RotationMode != EOrganicRotationMode::ControlDirection)
		{
			if (InputGait == EOrganicGait::Fast)
			{
				return EOrganicGait::Fast;
			}
			return InputGait;
		}
	}
	if (InputGait == EOrganicGait::Fast)
	{
		return EOrganicGait::Medium;
	}
	return InputGait;
}

EOrganicGait ABaseOrganic::CalculateActualGait(EOrganicGait AllowedGait) const
{
	float LocWalkSpeed = GetOrganicMovement()->MovementSettings.SlowSpeed;
	float LocRunSpeed = GetOrganicMovement()->MovementSettings.MediumSpeed;
	if (Speed > LocRunSpeed + 10.0f)
	{
		if (AllowedGait == EOrganicGait::Fast)
		{
			return EOrganicGait::Fast;
		}
		return EOrganicGait::Medium;
	}
	if (Speed >= LocWalkSpeed + 10.0f)
	{
		return EOrganicGait::Medium;
	}
	return EOrganicGait::Slow;
}

void ABaseOrganic::SetOverlayOverrideState(int32 NewState)
{
	OverlayOverrideState = NewState;
}

float ABaseOrganic::GetAnimCurveValue(FName CurveName) const
{
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}
	return 0.0f;
}

bool ABaseOrganic::CanSprint() const
{
	bool bCanBeFaster = false;
	if (!bHasMovementInput || RotationMode == EOrganicRotationMode::ControlDirection)
	{
		return bCanBeFaster;
	}

	bCanBeFaster = MovementInputAmount > 0.9f;
	if (RotationMode == EOrganicRotationMode::VelocityDirection)
	{
		return bCanBeFaster;
	}

	if (RotationMode == EOrganicRotationMode::LookingDirection)
	{
		const FRotator AccRot = GetOrganicMovement()->GetVelocity().ToOrientationRotator();
		FRotator Delta = AccRot - ViewRotation;
		Delta.Normalize();

		bool bResult = bCanBeFaster && FMath::Abs(Delta.Yaw) < 50.0f;
		return bResult;
	}
	return false;
}

void ABaseOrganic::ForceUpdateCharacterState()
{
	SetGait(InputGait, true);
	SetStance(InputStance, true);
	SetRotationMode(InputRotationMode, true);
	SetMovementState(MovementState, true);
}

void ABaseOrganic::OnMovementStateChanged(const EOrganicMovementState PreviousState)
{
	if (MovementState == EOrganicMovementState::Airborne)
	{
		AirborneRotation = OrganicMovement->GetRootCollisionRotation();
		if (Stance == EOrganicStance::Crouching)
		{
			OrganicMovement->OnUnCrouch();
		}
	}
}

void ABaseOrganic::SetInputRotationMode(EOrganicRotationMode NewInputRotationMode)
{
	InputRotationMode = NewInputRotationMode;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetInputRotationMode(NewInputRotationMode);
	}
}

void ABaseOrganic::Server_SetInputRotationMode_Implementation(EOrganicRotationMode NewInputRotationMode)
{
	SetInputRotationMode(NewInputRotationMode);
}

void ABaseOrganic::SetRotationMode(EOrganicRotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		const EOrganicRotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode, bForce);
		}
	}
}

void ABaseOrganic::Server_SetRotationMode_Implementation(EOrganicRotationMode NewRotationMode, bool bForce)
{
	SetRotationMode(NewRotationMode);
}

void ABaseOrganic::OnRotationModeChanged(EOrganicRotationMode PrevRotationMode)
{
	OrganicMovement->SetMovementSettings(GetMovementSettings());
}

void ABaseOrganic::SetInputStance(EOrganicStance NewInputStance)
{
	InputStance = NewInputStance;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetInputStance(NewInputStance);
	}
}

void ABaseOrganic::Server_SetInputStance_Implementation(EOrganicStance NewInputStance)
{
	SetInputStance(NewInputStance);
}

void ABaseOrganic::UpdateGroundRotation(float DeltaTime)
{
	if ((bIsMoving && bHasMovementInput) || Speed > 150.0f) // TODO: && !HasAnyRootMotion())
	{
		const float GroundedRotationRate = CalculateGroundRotationRate();
		if (RotationMode == EOrganicRotationMode::ControlDirection)
		{
			SmoothRotation({0.0f, ViewRotation.Yaw, 0.0f}, 1000.0f, 2.5f, DeltaTime);
		}
		else if (RotationMode == EOrganicRotationMode::LookingDirection)
		{
			float YawValue;
			if (Gait == EOrganicGait::Fast)
			{
				YawValue = LastVelocityRotation.Yaw;
			}
			else
			{
				float YawOffsetCurveVal = GetAnimCurveValue(NAME_YawOffset);
				YawValue = ViewRotation.Yaw + YawOffsetCurveVal;
			}
			SmoothRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
		}
		else if (RotationMode == EOrganicRotationMode::VelocityDirection)
		{
			SmoothRotation({0.0f, LastVelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate, DeltaTime);
		}
	}
	else
	{
		if (RotationMode == EOrganicRotationMode::ControlDirection)
		{
			LimitRotation(-ViewTurnLimit, ViewTurnLimit, 20.0f, DeltaTime);
		}
		else if (RotationMode == EOrganicRotationMode::LookingDirection)
		{
			float RotationAmount = GetAnimCurveValue(NAME_RotationAmount);
			if (FMath::Abs(RotationAmount) > 0.001f)
			{
				TargetRotation.Yaw = FRotator::NormalizeAxis(TargetRotation.Yaw + RotationAmount * (DeltaTime / (1.0f / 30.0f)));
				GetOrganicMovement()->SetRootCollisionRotation(TargetRotation);
			}
			// TODO: убедиться в надобности
			TargetRotation = GetOrganicMovement()->GetRootCollisionRotation();
		}
	}
}

void ABaseOrganic::UpdateAirborneRotation(float DeltaTime)
{
	if (RotationMode == EOrganicRotationMode::VelocityDirection || RotationMode == EOrganicRotationMode::LookingDirection)
	{
		SmoothRotation({0.0f, AirborneRotation.Yaw, 0.0f}, 0.0f, 2.5f, DeltaTime);
		AirborneRotation = Speed > 100.0 ? LastVelocityRotation : GetOrganicMovement()->GetRootCollisionRotation();
	}
	else if (RotationMode == EOrganicRotationMode:: ControlDirection)
	{
		SmoothRotation({0.0f, ViewRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		AirborneRotation = GetOrganicMovement()->GetRootCollisionRotation();
	}
}

void ABaseOrganic::SmoothRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime)
{
	TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	GetOrganicMovement()->SetRootCollisionRotation(FMath::RInterpTo(GetOrganicMovement()->GetRootCollisionRotation(),
	                                                                TargetRotation, DeltaTime, ActorInterpSpeed));
}

void ABaseOrganic::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	FRotator AimDelta = ViewRotation - GetOrganicMovement()->GetRootCollisionRotation();
	AimDelta.Normalize();
	
	float RangeValue = AimDelta.Yaw;
	if (RangeValue < AimYawMin || RangeValue > AimYawMax)
	{
		float ControlRotYaw = ViewRotation.Yaw;
		float TargetYaw = ControlRotYaw + (RangeValue > 0.0f ? AimYawMin : AimYawMax);
		
		SmoothRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

float ABaseOrganic::CalculateGroundRotationRate() const
{
	FOrganicMovementSettings MovementSettings = GetOrganicMovement()->MovementSettings;
	float MappedSpeed = GetOrganicMovement()->GetMappedSpeed();
	
	float CurveValue = MovementSettings.RotationRateCurve->GetFloatValue(MappedSpeed);
	float MappedViewYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {0.5f, 1.0f}, ViewYawRate);
	return CurveValue * MappedViewYawRate;
}

void ABaseOrganic::OnSprint(bool bEnabled)
{
	bEnabled ? SetInputGait(EOrganicGait::Fast) : SetInputGait(EOrganicGait::Medium);
}

void ABaseOrganic::OnCrouch()
{
	SetStance(EOrganicStance::Crouching);
}

void ABaseOrganic::OnUnCrouch()
{
	SetStance(EOrganicStance::Standing);
}

void ABaseOrganic::OnJump()
{
	if (MovementState == EOrganicMovementState::Ground)
	{
		if (Stance == EOrganicStance::Standing)
		{
			OrganicMovement->OnJump();
			OnJumpedDelegate.Broadcast();
		}
		else if (Stance == EOrganicStance::Crouching)
		{
			OrganicMovement->OnUnCrouch();
		}
	}
}

bool ABaseOrganic::IsSprinting() const
{
	return GetGait() == EOrganicGait::Fast;
}

bool ABaseOrganic::IsAirborne() const
{
	return GetMovementState() == EOrganicMovementState::Airborne;
}
