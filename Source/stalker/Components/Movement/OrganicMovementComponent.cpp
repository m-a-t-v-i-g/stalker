// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Movement/OrganicMovementComponent.h"

#include "MovementModelConfig.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Organic/BaseOrganic.h"

UOrganicMovementComponent::UOrganicMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	TargetMaxSpeed = MaxDesiredSpeed;
}

void UOrganicMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);
	OrganicOwner = GetOwner<ABaseOrganic>();
}

void UOrganicMovementComponent::BindReplicationData_Implementation()
{
	Super::BindReplicationData_Implementation();

	BindInputFlag(ID_Action3(), bWantsToSprint);
	BindInputFlag(ID_Action2(), bWantsToCrouch);
	BindInputFlag(ID_Action1(), bWantsToJump);
	
	BindRotator(ViewRotation, false, true, true);
	BindRotator(PrevPawnRotation, false, true, true);

	BindBool(bJustSprinting, false, true, true);
	BindBool(bJustJumped, false, true, true);
	BindBool(bJustCrouched, false, true, true);
}

void UOrganicMovementComponent::PhysicsGrounded(float DeltaSeconds)
{
	Super::PhysicsGrounded(DeltaSeconds);
}

void UOrganicMovementComponent::PhysicsAirborne(float DeltaSeconds)
{
	Super::PhysicsAirborne(DeltaSeconds);
}

void UOrganicMovementComponent::PreMovementUpdate_Implementation(float DeltaSeconds)
{
	Super::PreMovementUpdate_Implementation(DeltaSeconds);
	
	if (GetIterationNumber() == 1)
	{
		bJustJumped = false;
	}
}

void UOrganicMovementComponent::PerformMovement(float DeltaSeconds)
{
	Super::PerformMovement(DeltaSeconds);
}

void UOrganicMovementComponent::MovementUpdate_Implementation(float DeltaSeconds)
{
	Super::MovementUpdate_Implementation(DeltaSeconds);

	PrevPawnRotation = GetInRotation();
	PrevComponentRotation = GetRootCollisionRotation();

	ViewRotation = FMath::RInterpTo(ViewRotation, GetInControlRotation(), DeltaSeconds, ViewInterpSpeed);

	const FVector PrevAcceleration = (GetVelocity() - PreviousVelocity) / DeltaSeconds;
	InstantAcceleration = PrevAcceleration.IsNearlyZero() ? InstantAcceleration / 2 : PrevAcceleration;

	float Speed = GetSpeedXY();
	bIsMoving = Speed > 1.0f;
	
	if (bIsMoving)
	{
		VelocityRotation = GetVelocity().ToOrientationRotator();
	}

	MovementInputValue = GetVelocity().Size2D() / GetMaxSpeed();
	bHasMovementInput = MovementInputValue > 0.0f;
	ViewYawRate = FMath::Abs((ViewRotation.Yaw - PreviousViewYaw) / DeltaSeconds);

	if (MovementStatus.Grounded())
	{
		UpdateGait();
		UpdateGroundRotation(DeltaSeconds);
	}
	else if (MovementStatus.Airborne())
	{
		UpdateAirborneRotation(DeltaSeconds);
	}
	
	if (bWantsChangeMovementSettings)
	{
		TargetMaxSpeed = MovementSettings.GetSpeedForGait(AllowedGait);
		bWantsChangeMovementSettings = false;
	}
	
	MaxDesiredSpeed = FMath::FInterpTo(MaxDesiredSpeed, TargetMaxSpeed, DeltaSeconds, SpeedInterpSpeed);
	
	PreviousVelocity = GetVelocity();
	PreviousViewYaw = ViewRotation.Yaw;
	
	UpdateSprint(bWantsToSprint);
	UpdateCrouch(bWantsToCrouch);
	UpdateJump(bWantsToJump);
}

void UOrganicMovementComponent::MovementUpdateSimulated_Implementation(float DeltaSeconds)
{
	Super::MovementUpdateSimulated_Implementation(DeltaSeconds);

	if (bWantsChangeMovementSettings)
	{
		TargetMaxSpeed = MovementSettings.GetSpeedForGait(AllowedGait);
		bWantsChangeMovementSettings = false;
	}

	MaxDesiredSpeed = FMath::FInterpTo(MaxDesiredSpeed, TargetMaxSpeed, DeltaSeconds, SpeedInterpSpeed);
	
	PrevComponentRotation = GetRootCollisionRotation();

	Sprinting();
	Crouching();
	Jumping();
}

void UOrganicMovementComponent::OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode)
{
	Super::OnMovementModeUpdated_Implementation(PreviousMovementMode);
	
	if (IsMovingOnGround())
	{
		SetMovementState(EOrganicMovementState::Ground);
	}
	else if (IsAirborne())
	{
		SetMovementState(EOrganicMovementState::Airborne);
	}
}

void UOrganicMovementComponent::OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode)
{
	Super::OnMovementModeChangedSimulated_Implementation(PreviousMovementMode);
	
	if (IsMovingOnGround())
	{
		SetMovementState(EOrganicMovementState::Ground);
	}
	else if (IsAirborne())
	{
		SetMovementState(EOrganicMovementState::Airborne);
	}
}

void UOrganicMovementComponent::SetMovementModel(UMovementModelConfig* ModelConfig)
{
	MovementModel = ModelConfig;
	ForceUpdateAllStates();
}

void UOrganicMovementComponent::SetMovementSettings(FMovementModel_Settings NewMovementSettings)
{
	MovementSettings = NewMovementSettings;
	bWantsChangeMovementSettings = true;
}

FMovementModel_Settings UOrganicMovementComponent::GetMovementSettings() const
{
	switch (RotationMode.RotationMode)
	{
	case EOrganicRotationMode::ControlDirection:
		{
			if (Stance == EOrganicStance::Standing)
			{
				return MovementModel->ControlDirection.Standing;
			}
			if (Stance == EOrganicStance::Crouching)
			{
				return MovementModel->ControlDirection.Crouching;
			}
		}
	case EOrganicRotationMode::LookingDirection:
		{
			if (Stance == EOrganicStance::Standing)
			{
				return MovementModel->LookingDirection.Standing;
			}
			if (Stance == EOrganicStance::Crouching)
			{
				return MovementModel->LookingDirection.Crouching;
			}
		}
	case EOrganicRotationMode::VelocityDirection:
		{
			if (Stance == EOrganicStance::Standing)
			{
				return MovementModel->VelocityDirection.Standing;
			}
			if (Stance == EOrganicStance::Crouching)
			{
				return MovementModel->VelocityDirection.Crouching;
			}
		}
	default: return MovementModel->VelocityDirection.Standing;
	}
}

void UOrganicMovementComponent::SetMovementState(const EOrganicMovementState NewMovementState, bool bForce)
{
	if (bForce || MovementStatus != NewMovementState)
	{
		PrevMovementState = MovementStatus;
		MovementStatus = NewMovementState;
		OnMovementStateChanged(PrevMovementState);
		
		UKismetSystemLibrary::PrintString(this, FString("Set state"), true, false, FLinearColor::Red, 0.5f, "ggg");
	}
}

void UOrganicMovementComponent::OnMovementStateChanged(const EOrganicMovementState PreviousState)
{
	if (MovementStatus.Airborne())
	{
		TargetRotation = GetRootCollisionRotation();
		if (Stance == EOrganicStance::Crouching)
		{
			OnUnCrouch();
		}
	}
}

void UOrganicMovementComponent::SetInputRotationMode(EOrganicRotationMode NewInputRotationMode)
{
	InputRotationMode = NewInputRotationMode;
}

void UOrganicMovementComponent::SetRotationMode(EOrganicRotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		const EOrganicRotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);
	}
}

void UOrganicMovementComponent::OnRotationModeChanged(EOrganicRotationMode PrevRotationMode)
{
	SetMovementSettings(GetMovementSettings());
}

void UOrganicMovementComponent::SetInputStance(EOrganicStance NewInputStance)
{
	InputStance = NewInputStance;
}

void UOrganicMovementComponent::SetStance(const EOrganicStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const EOrganicStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void UOrganicMovementComponent::OnStanceChanged(EOrganicStance PreviousStance)
{
	SetMovementSettings(GetMovementSettings());
}

void UOrganicMovementComponent::SetInputGait(EOrganicGait NewInputGait)
{
	InputGait = NewInputGait;
}

void UOrganicMovementComponent::SetGait(const EOrganicGait NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		const EOrganicGait PreviousGait = Gait;
		Gait = NewGait;
		OnGaitChanged(PreviousGait);
	}
}

void UOrganicMovementComponent::OnGaitChanged(EOrganicGait PreviousGait)
{
	
}

void UOrganicMovementComponent::ForceUpdateAllStates()
{
	SetMovementState(MovementStatus, true);
	SetRotationMode(InputRotationMode, true);
	SetStance(InputStance, true);
	SetGait(InputGait, true);
}

void UOrganicMovementComponent::UpdateGait()
{
	const EOrganicGait CalcAllowedGait = CalculateAllowedGait();
	const EOrganicGait CalcActualGait = CalculateActualGait(CalcAllowedGait);

	if (CalcActualGait != Gait)
	{
		SetGait(CalcActualGait);
	}
	SetAllowedGait(CalcAllowedGait);
}

EOrganicGait UOrganicMovementComponent::CalculateAllowedGait() const
{
	if (Stance.Standing())
	{
		if (!RotationMode.ControlDirection())
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

EOrganicGait UOrganicMovementComponent::CalculateActualGait(EOrganicGait NewAllowedGait) const
{
	float Speed = GetSpeedXY();
	if (Speed > MovementSettings.MediumSpeed + 10.0f)
	{
		if (NewAllowedGait == EOrganicGait::Fast)
		{
			return EOrganicGait::Fast;
		}
		return EOrganicGait::Medium;
	}
	
	if (Speed >= MovementSettings.SlowSpeed + 10.0f)
	{
		return EOrganicGait::Medium;
	}
	
	return EOrganicGait::Slow;
}

void UOrganicMovementComponent::SetAllowedGait(EOrganicGait DesiredGait)
{
	if (AllowedGait == DesiredGait) return;

	AllowedGait = DesiredGait;
	bWantsChangeMovementSettings = true;
}

float UOrganicMovementComponent::CalculateGroundRotationRate() const
{
	float CurveValue = MovementSettings.RotationRateCurve->GetFloatValue(GetMappedSpeed());
	float MappedViewYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {0.5f, 1.0f}, ViewYawRate);
	return CurveValue * MappedViewYawRate;
}

float UOrganicMovementComponent::GetAnimCurveValue(FName CurveName) const
{
	if (OrganicOwner->GetMesh() && OrganicOwner->GetMesh()->GetAnimInstance())
	{
		return OrganicOwner->GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}
	return 0.0f;
}

float UOrganicMovementComponent::GetMappedSpeed() const
{
	float Speed = Velocity.Size2D();
	
	float LocWalkSpeed = MovementSettings.SlowSpeed;
	float LocRunSpeed = MovementSettings.MediumSpeed;
	float LocSprintSpeed = MovementSettings.FastSpeed;
	
	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}
	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}
	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UOrganicMovementComponent::UpdateGroundRotation(float DeltaTime)
{
	if ((bIsMoving && bHasMovementInput) || GetSpeedXY() > 150.0f) // TODO: && !HasAnyRootMotion())
	{
		const float GroundedRotationRate = CalculateGroundRotationRate();
		if (RotationMode.ControlDirection())
		{
			RotateRootCollision({0.0f, ViewRotation.Yaw, 0.0f}, 1000.0f, 2.5f, DeltaTime);
		}
		else if (RotationMode.LookingDirection())
		{
			float YawValue;
			if (Gait.Sprinting())
			{
				YawValue = VelocityRotation.Yaw;
			}
			else
			{
				float YawOffsetCurveVal = GetAnimCurveValue("YawOffset");
				YawValue = ViewRotation.Yaw + YawOffsetCurveVal;
			}
			RotateRootCollision({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
		}
		else if (RotationMode.VelocityDirection())
		{
			RotateRootCollision({0.0f, VelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate, DeltaTime);
		}
	}
	else
	{
		if (RotationMode.ControlDirection())
		{
			// TODO: убрать хард код
			LimitRotation(-35.0f, 35.0f, 20.0f, DeltaTime);
		}
		else if (RotationMode.LookingDirection())
		{
			float RotationAmount = GetAnimCurveValue("RotationAmount");
			if (FMath::Abs(RotationAmount) > 0.001f)
			{
				TargetRotation.Yaw = FRotator::NormalizeAxis(TargetRotation.Yaw + RotationAmount * (DeltaTime / (1.0f / 30.0f)));
				SetRootCollisionRotation(TargetRotation);
			}
			TargetRotation = GetRootCollisionRotation();
		}
	}
}

void UOrganicMovementComponent::UpdateAirborneRotation(float DeltaTime)
{
	if (RotationMode.VelocityDirection() || RotationMode.LookingDirection())
	{
		RotateRootCollision({0.0f, TargetRotation.Yaw, 0.0f}, 0.0f, 2.5f, DeltaTime);
		TargetRotation = GetSpeedXY() > 100.0 ? VelocityRotation : GetRootCollisionRotation();
	}
	else if (RotationMode == EOrganicRotationMode:: ControlDirection)
	{
		RotateRootCollision({0.0f, ViewRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		TargetRotation = GetRootCollisionRotation();
	}
}

void UOrganicMovementComponent::RotateRootCollision(const FRotator& Target, float TargetInterpSpeed,
                                                    float ActorInterpSpeed, float DeltaTime)
{
	TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetRootCollisionRotation(FMath::RInterpTo(GetRootCollisionRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

void UOrganicMovementComponent::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	FRotator AimDelta = ViewRotation - GetRootCollisionRotation();
	AimDelta.Normalize();
	
	float RangeValue = AimDelta.Yaw;
	
	if (RangeValue < AimYawMin || RangeValue > AimYawMax)
	{
		float ControlRotYaw = ViewRotation.Yaw;
		float TargetYaw = ControlRotYaw + (RangeValue > 0.0f ? AimYawMin : AimYawMax);
		
		RotateRootCollision({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

void UOrganicMovementComponent::UpdateSprint(bool bRequestedSprint)
{
	if (bRequestedSprint)
	{
		if (CanSprint())
		{
			bJustSprinting = true;
		}
	}
	else
	{
		bJustSprinting = false;
		bCanSprint = true;
	}
	Sprinting();
}

void UOrganicMovementComponent::UpdateCrouch(bool bRequestedCrouch)
{
	if (bRequestedCrouch)
	{
		if (CanCrouch() && !bJustCrouched)
		{
			OnCrouch();
		}
		else if (CanCrouch() && bJustCrouched)
		{
			OnUnCrouch();
		}
		bCanCrouch = false;
	}
	else
	{
		bCanCrouch = true;
	}
	Crouching();
}

void UOrganicMovementComponent::UpdateJump(bool bRequestedJump)
{
	if (bRequestedJump)
	{
		if (CanJump())
		{
			bJustJumped = true;
		}
		bCanJump = false;
	}
	else
	{
		bCanJump = true;
	}
	Jumping();
}

void UOrganicMovementComponent::Sprinting()
{
	if (bJustSprinting)
	{
		OrganicOwner->OnSprint(true);
	}
	else
	{
		OrganicOwner->OnSprint(false);
	}
}

void UOrganicMovementComponent::Crouching()
{
	if (bJustCrouched)
	{
		OrganicOwner->OnCrouch();
	}
	else
	{
		OrganicOwner->OnUnCrouch();
	}
}

void UOrganicMovementComponent::Jumping()
{
	if (bJustJumped)
	{
		OrganicOwner->OnJump();
	}
}

bool UOrganicMovementComponent::CanSprint() const
{
	bool bCannotSprint = !IsMovingOnGround() || IsFalling() || bWantsToJump || bWantsToCrouch;
	if (!bCannotSprint)
	{
		if (!bHasMovementInput || RotationMode == EOrganicRotationMode::ControlDirection)
		{
			return false;
		}

		if (RotationMode == EOrganicRotationMode::VelocityDirection)
		{
			return MovementInputValue > 0.9f;
		}

		if (RotationMode == EOrganicRotationMode::LookingDirection)
		{
			const FRotator AccRot = GetVelocity().ToOrientationRotator();
			FRotator Delta = AccRot - ViewRotation;
			Delta.Normalize();

			return FMath::Abs(Delta.Yaw) < 50.0f;
		}
		return false;
	}
	return bCanSprint & !bCannotSprint;
}

bool UOrganicMovementComponent::CanCrouch() const
{
	bool bCannotCrouch = !CanEverCrouch() || !IsMovingOnGround() || IsFalling() || bWantsToJump || bWantsToSprint;
	return bCanCrouch & !bCannotCrouch;
}

bool UOrganicMovementComponent::CanJump() const
{
	bool bCannotJump = !CanEverJump() || !IsMovingOnGround() || bWantsToCrouch;
	return bCanJump & !bCannotJump;
}

void UOrganicMovementComponent::OnJump()
{
	AddImpulse({0.0f, 0.0f, JumpForce}, true);
}

void UOrganicMovementComponent::OnCrouch()
{
	bJustCrouched = true;
}

void UOrganicMovementComponent::OnUnCrouch()
{
	bJustCrouched = false;
}
