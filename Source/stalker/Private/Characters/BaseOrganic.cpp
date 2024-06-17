// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BaseOrganic.h"
#include "GenCapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"

FName ABaseOrganic::CapsuleName {"Collision Cylinder"};
FName ABaseOrganic::OrganicMovementName {"CharMoveComp"};
FName ABaseOrganic::MeshName {"CharacterMesh0"};

ABaseOrganic::ABaseOrganic()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CapsuleComponent = CreateDefaultSubobject<UGenCapsuleComponent>(CapsuleName);
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = FName(TEXT("Characters"));
		ArrowComponent->SpriteInfo.DisplayName = FText(NSLOCTEXT("SpriteCategory", "Characters", "Characters"));
		ArrowComponent->SetupAttachment(CapsuleComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif

	OrganicMovement = CreateDefaultSubobject<UOrganicMovementComponent>(OrganicMovementName);
	if (OrganicMovement)
	{
		OrganicMovement->UpdatedComponent = CapsuleComponent;
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
		static FName MeshCollisionProfileName(TEXT("CharacterMesh"));
		Mesh->SetCollisionProfileName(MeshCollisionProfileName);
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SetReplicatingMovement(false);
}

void ABaseOrganic::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABaseOrganic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Mesh)
	{
		if (Mesh->PrimaryComponentTick.bCanEverTick && OrganicMovement)
		{
			Mesh->PrimaryComponentTick.AddPrerequisite(OrganicMovement, OrganicMovement->PrimaryComponentTick);
		}
	}
	if (OrganicMovement && CapsuleComponent)
	{
		OrganicMovement->UpdateNavAgent(*CapsuleComponent);
	}
}

void ABaseOrganic::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseOrganic::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	OrganicTick(DeltaSeconds);
}

void ABaseOrganic::OnMovementModeChanged()
{
	if (OrganicMovement->IsMovingOnGround())
	{
		SetMovementState(EOrganicMovementState::Ground);
	}
	else if (OrganicMovement->IsAirborne())
	{
		SetMovementState(EOrganicMovementState::Airborne);
	}
}

void ABaseOrganic::OrganicTick(float DeltaTime)
{
	Acceleration = OrganicMovement->GetTransientAcceleration();
	ViewRotation = OrganicMovement->GetViewRotation();
	
	const FVector Velocity = OrganicMovement->GetVelocity();
	
	//const FVector NewAcceleration = (Velocity - PreviousVelocity) / DeltaTime; TODO: Old acceleration
	//Acceleration = NewAcceleration.IsNearlyZero() ? Acceleration / 2 : NewAcceleration;
	
	Speed = Velocity.Size2D();
	bIsMoving = Speed > 1.0f;
	if (bIsMoving)
	{
		LastVelocityRotation = Velocity.ToOrientationRotator();
	}

	float InputAcceleration = GetOrganicMovement()->GetInputAcceleration();
	MovementInputAmount = Acceleration.Size() / InputAcceleration;
	
	bHasMovementInput = MovementInputAmount > 0.0f;
	if (bHasMovementInput)
	{
		LastInputRotation = Acceleration.ToOrientationRotator();
	}
	ViewYawRate = FMath::Abs((ViewRotation.Yaw - PreviousViewYaw) / DeltaTime);
	
	if (MovementState == EOrganicMovementState::Ground)
	{
		
	}
	else if (MovementState == EOrganicMovementState::Airborne)
	{
		
	}
	else if (MovementState == EOrganicMovementState::Ragdoll)
	{
		
	}
	
	PreviousVelocity = GetVelocity();
	PreviousViewYaw = ViewRotation.Yaw;
}

void ABaseOrganic::SetMovementModel()
{
	const FString ContextString = GetFullName();
	const FMovementStateSettings* OutRow = MovementTable.DataTable->FindRow<FMovementStateSettings>(MovementTable.RowName, ContextString);
	
	MovementModel = *OutRow;
}

FMovementSettings ABaseOrganic::GetMovementSettings() const
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
	//OrganicMovement->SetMovementSettings(GetMovementSettings()); TODO
}

void ABaseOrganic::GaitTick()
{
	const EOrganicGait AllowedGait = CalculateAllowedGait();
	const EOrganicGait ActualGait = CalculateActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}
	//OrganicMovement->SetAllowedGait(AllowedGait); TODO!!!
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
				return CanSprint() ? EOrganicGait::Fast : EOrganicGait::Medium;
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
	float LocWalkSpeed = OrganicMovement->CurrentMovementSettings.WalkSpeed;
	float LocRunSpeed = OrganicMovement->CurrentMovementSettings.RunSpeed;
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

void ABaseOrganic::SetMovementAction(EOrganicMovementAction NewMovementAction, bool bForce)
{
	if (bForce || MovementAction != NewMovementAction)
	{
		const EOrganicMovementAction Prev = MovementAction;
		MovementAction = NewMovementAction;
		OnMovementActionChanged(Prev);
	}
}

void ABaseOrganic::OnMovementStateChanged(const EOrganicMovementState PreviousState)
{
	if (MovementState == EOrganicMovementState::Airborne)
	{
		/*
		if (MovementAction == EWMovementAction::None)
		{
			AirborneRotation = OrganicMovement->GetRootCollisionRotation();
			if (Stance == EWStanceType::Crouching)
			{
				//OrganicMovement->OnUncrouch(true); TODO: For character
			}
		}
		else if (MovementAction == EWMovementAction::Rolling)
		{
			//RagdollStart(); TODO: For character
		}
		*/
	}
}

void ABaseOrganic::OnMovementActionChanged(const EOrganicMovementAction PreviousAction)
{
	if (MovementAction == EOrganicMovementAction::Rolling)
	{
		//CharacterMovement->OnCrouch(true); TODO
	}
	if (PreviousAction == EOrganicMovementAction::Rolling)
	{
		if (InputStance == EOrganicStance::Standing)
		{
			//CharacterMovement->OnUncrouch(true); TODO
		}
		else if (InputStance == EOrganicStance::Crouching)
		{
			//CharacterMovement->OnCrouch(true); TODO
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
	//OrganicMovement->SetMovementSettings(GetMovementSettings()); TODO
}

void ABaseOrganic::OnRep_RotationMode(EOrganicRotationMode PrevRotationMode)
{
	OnRotationModeChanged(PrevRotationMode);
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
	
}

void ABaseOrganic::UpdateAirborneRotation(float DeltaTime)
{
	
}
