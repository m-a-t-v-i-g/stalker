// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BaseOrganic.h"
#include "GenCapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

FName ABaseOrganic::CapsuleName {"Collision Cylinder"};
FName ABaseOrganic::OrganicMovementName {"OrganicMoveComp"};
FName ABaseOrganic::MeshName {"OrganicMesh0"};

const FName NAME_RotationAmount(TEXT("RotationAmount"));
const FName NAME_YawOffset(TEXT("YawOffset"));

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

	if (GetMesh())
	{
		GetMesh()->AddTickPrerequisiteActor(this);
	}
	
	SetMovementModel();
	ForceUpdateCharacterState();

	TargetRotation = GetOrganicMovement()->GetRootCollisionRotation();
	LastVelocityRotation = TargetRotation;
	LastInputRotation = TargetRotation;

	GetOrganicMovement()->SetMovementSettings(GetMovementSettings());
}

void ABaseOrganic::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	OrganicTick(DeltaSeconds);
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

void ABaseOrganic::OrganicTick(float DeltaTime)
{
	Acceleration = GetOrganicMovement()->GetTransientAcceleration();
	ViewRotation = GetOrganicMovement()->GetViewRotation();
	
	const FVector Velocity = GetOrganicMovement()->GetVelocity();
	
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
		const EOrganicGait AllowedGait = CalculateAllowedGait();
		const EOrganicGait ActualGait = CalculateActualGait(AllowedGait);

		if (ActualGait != Gait)
		{
			SetGait(ActualGait);
		}
		
		GetOrganicMovement()->SetAllowedGait(AllowedGait);
		UpdateGroundRotation(DeltaTime);
	}
	else if (MovementState == EOrganicMovementState::Airborne)
	{
		UpdateAirborneRotation(DeltaTime);
	}
	else if (MovementState == EOrganicMovementState::Ragdoll)
	{
		// UpdateRagdoll;
	}
	
	PreviousVelocity = GetVelocity();
	PreviousViewYaw = ViewRotation.Yaw;
}

void ABaseOrganic::SetMovementModel()
{
	const FString ContextString = GetFullName();
	const FOrganicMovementStateSettings* OutRow = MovementTable.DataTable->FindRow<FOrganicMovementStateSettings>(
		MovementTable.RowName, ContextString);
	
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
				return CanBeFaster() ? EOrganicGait::Fast : EOrganicGait::Medium;
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

void ABaseOrganic::ForceUpdateCharacterState()
{
	SetGait(InputGait, true);
	SetStance(InputStance, true);
	SetRotationMode(InputRotationMode, true);
	SetMovementState(MovementState, true);
}

float ABaseOrganic::GetAnimCurveValue(FName CurveName) const
{
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}
	return 0.0;
}

bool ABaseOrganic::CanBeFaster() const
{
	if (!bHasMovementInput || RotationMode == EOrganicRotationMode::ControlDirection)
	{
		return false;
	}

	const bool bValidInputAmount = MovementInputAmount > 0.9f;
	if (RotationMode == EOrganicRotationMode::VelocityDirection)
	{
		return bValidInputAmount;
	}

	if (RotationMode == EOrganicRotationMode::LookingDirection)
	{
		const FRotator AccRot = Acceleration.ToOrientationRotator();
		FRotator Delta = AccRot - ViewRotation;
		Delta.Normalize();

		return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
	}
	return false;
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
	if ((bIsMoving && bHasMovementInput) || Speed > 150.0f) // && !HasAnyRootMotion())
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
				TargetRotation.Yaw = UKismetMathLibrary::NormalizeAxis(
					TargetRotation.Yaw + RotationAmount * (DeltaTime / (1.0f / 30.0f)));
				GetOrganicMovement()->SetRootCollisionRotation(TargetRotation);
			}
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
	float MappedSpeedVal = GetOrganicMovement()->GetMappedSpeed();
	float CurveVal = GetOrganicMovement()->MovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
	float ClampedAimYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {0.5f, 1.0f}, ViewYawRate);
	return CurveVal * ClampedAimYawRate;
}
