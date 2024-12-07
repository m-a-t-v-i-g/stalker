// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacterMovementComponent.h"
#include "MovementModelConfig.h"
#include "StalkerCharacter.h"

UStalkerCharacterMovementComponent::UStalkerCharacterMovementComponent()
{
	InputRotationMode = ECharacterRotationMode::LookingDirection;
	TargetMaxSpeed = MaxDesiredSpeed;
	InputAccelerationGrounded = 2200.0f;
	OverMaxSpeedDecelerationAirborne = 3000.0f;
	bOrientToInputDirection = false;
	RotationRate = 100.0f;
}

void UStalkerCharacterMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);

	if (auto StalkerChar = GetOwner<AStalkerCharacter>())
	{
		CharacterOwner = StalkerChar;
	}
}

void UStalkerCharacterMovementComponent::BindReplicationData_Implementation()
{
	Super::BindReplicationData_Implementation();

	BindInputFlag(ID_Action3(), bWantsToSprint);
	BindInputFlag(ID_Action2(), bWantsToCrouch);
	BindInputFlag(ID_Action1(), bWantsToJump);
	
	BindBool(bJustSprinting, false, true, true);
	BindBool(bJustJumped, false, true, true);
	BindBool(bJustCrouched, false, true, true);
	
	BindRotator(ViewRotation, false, true, true);
	BindRotator(PrevCharacterRotation, false, true, true);
}

void UStalkerCharacterMovementComponent::PreMovementUpdate_Implementation(float DeltaSeconds)
{
	Super::PreMovementUpdate_Implementation(DeltaSeconds);
	
	if (GetIterationNumber() == 1)
	{
		bJustJumped = false;
	}
}

void UStalkerCharacterMovementComponent::MovementUpdate_Implementation(float DeltaSeconds)
{
	Super::MovementUpdate_Implementation(DeltaSeconds);

	CalculateMovement(DeltaSeconds);
	
	UpdateSprint(bWantsToSprint);
	UpdateCrouch(bWantsToCrouch);
	UpdateJump(bWantsToJump);
}

void UStalkerCharacterMovementComponent::MovementUpdateSimulated_Implementation(float DeltaSeconds)
{
	Super::MovementUpdateSimulated_Implementation(DeltaSeconds);

	CalculateMovement(DeltaSeconds);
	
	Sprinting();
	Crouching();
	Jumping();
}

void UStalkerCharacterMovementComponent::OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode)
{
	Super::OnMovementModeUpdated_Implementation(PreviousMovementMode);
	
	if (IsMovingOnGround())
	{
		SetMovementState(ECharacterMovementState::Ground);
	}
	else if (IsAirborne())
	{
		SetMovementState(ECharacterMovementState::Airborne);
	}
}

void UStalkerCharacterMovementComponent::OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode)
{
	Super::OnMovementModeChangedSimulated_Implementation(PreviousMovementMode);
	
	if (IsMovingOnGround())
	{
		SetMovementState(ECharacterMovementState::Ground);
	}
	else if (IsAirborne())
	{
		SetMovementState(ECharacterMovementState::Airborne);
	}
}

void UStalkerCharacterMovementComponent::SetMovementModel(const UMovementModelConfig* ModelConfig)
{
	CurrentMovementConfig = ModelConfig;
	ForceUpdateAllStates();
}

void UStalkerCharacterMovementComponent::SetMovementSettings()
{
	MovementModel = CurrentMovementConfig->GetMovementSettings(RotationMode.RotationMode, Stance.Stance);
}

void UStalkerCharacterMovementComponent::SetMovementState(const ECharacterMovementState NewMovementState, bool bForce)
{
	if (bForce || CharMovementState != NewMovementState)
	{
		PrevMovementState = CharMovementState;
		CharMovementState = NewMovementState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void UStalkerCharacterMovementComponent::OnMovementStateChanged(const ECharacterMovementState PreviousState)
{
	if (CharMovementState.Airborne())
	{
		TargetRotation = GetRootCollisionRotation();
		
		if (Stance == ECharacterStanceType::Crouching)
		{
			OnUnCrouch();
		}
	}
}

void UStalkerCharacterMovementComponent::SetInputRotationMode(ECharacterRotationMode NewInputRotationMode)
{
	InputRotationMode = NewInputRotationMode;
}

void UStalkerCharacterMovementComponent::SetRotationMode(ECharacterRotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		const ECharacterRotationMode Prev = RotationMode.RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);
	}
}

void UStalkerCharacterMovementComponent::OnRotationModeChanged(ECharacterRotationMode PrevRotationMode)
{
	if (RotationMode != PrevRotationMode)
	{
		SetMovementSettings();
	}
}

void UStalkerCharacterMovementComponent::SetInputStance(ECharacterStanceType NewInputStance)
{
	InputStance = NewInputStance;
}

void UStalkerCharacterMovementComponent::SetStance(const ECharacterStanceType NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const ECharacterStanceType Prev = Stance.Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void UStalkerCharacterMovementComponent::OnStanceChanged(ECharacterStanceType PreviousStance)
{
	if (Stance != PreviousStance)
	{
		SetMovementSettings();
	}
}

void UStalkerCharacterMovementComponent::SetInputGait(ECharacterGaitType NewInputGait)
{
	InputGait = NewInputGait;
}

void UStalkerCharacterMovementComponent::SetGait(const ECharacterGaitType NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		const ECharacterGaitType PreviousGait = Gait;
		Gait = NewGait;
		OnGaitChanged(PreviousGait);
	}
}

void UStalkerCharacterMovementComponent::OnGaitChanged(ECharacterGaitType PreviousGait)
{
	if (Gait != PreviousGait)
	{
		// Logic
	}
}

void UStalkerCharacterMovementComponent::UpdateGait()
{
	const ECharacterGaitType CalcAllowedGait = CalculateAllowedGait();
	const ECharacterGaitType CalcActualGait = CalculateActualGait(CalcAllowedGait);

	if (CalcActualGait != Gait)
	{
		SetGait(CalcActualGait);
	}

	if (AllowedGait != CalcAllowedGait)
	{
		AllowedGait = CalcAllowedGait;
		TargetMaxSpeed = MovementModel.GetSpeedForGait(AllowedGait);
	}
}

ECharacterGaitType UStalkerCharacterMovementComponent::CalculateAllowedGait() const
{
	if (Stance.Standing())
	{
		if (!RotationMode.ControlDirection())
		{
			if (InputGait == ECharacterGaitType::Sprint)
			{
				return ECharacterGaitType::Sprint;
			}
			return InputGait;
		}
	}
	if (InputGait == ECharacterGaitType::Sprint)
	{
		return ECharacterGaitType::Run;
	}
	return InputGait;
}

ECharacterGaitType UStalkerCharacterMovementComponent::CalculateActualGait(ECharacterGaitType NewAllowedGait) const
{
	float Speed = GetSpeedXY();
	
	if (Speed > MovementModel.RunSpeed + 10.0f)
	{
		if (NewAllowedGait == ECharacterGaitType::Sprint)
		{
			return ECharacterGaitType::Sprint;
		}
		return ECharacterGaitType::Run;
	}
	
	if (Speed >= MovementModel.WalkSpeed + 10.0f)
	{
		return ECharacterGaitType::Run;
	}
	
	return ECharacterGaitType::Walk;
}

void UStalkerCharacterMovementComponent::ForceUpdateAllStates()
{
	SetMovementState(CharMovementState, true);
	SetRotationMode(InputRotationMode, true);
	SetStance(InputStance, true);
	SetGait(InputGait, true);
	SetMovementSettings();
}

float UStalkerCharacterMovementComponent::CalculateGroundRotationRate() const
{
	float CurveValue = MovementModel.RotationCurve->GetFloatValue(GetMappedSpeed());
	float MappedViewYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {0.5f, 1.0f}, ViewYawRate);
	return CurveValue * MappedViewYawRate;
}

float UStalkerCharacterMovementComponent::GetAnimCurveValue(FName CurveName) const
{
	if (CharacterOwner->GetMesh() && CharacterOwner->GetMesh()->GetAnimInstance())
	{
		return CharacterOwner->GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}
	return 0.0f;
}

float UStalkerCharacterMovementComponent::GetMappedSpeed() const
{
	float Speed = GetSpeedXY();
	
	float LocWalkSpeed = MovementModel.WalkSpeed;
	float LocRunSpeed = MovementModel.RunSpeed;
	float LocSprintSpeed = MovementModel.SprintSpeed;
	
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

void UStalkerCharacterMovementComponent::UpdateGroundRotation(float DeltaTime)
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
			LimitRotation(-50.0f, 50.0f, 20.0f, DeltaTime);
		}

		float RotationAmount = GetAnimCurveValue("RotationAmount");
		if (FMath::Abs(RotationAmount) > 0.001f)
		{
			TargetRotation.Yaw = FRotator::NormalizeAxis(
				TargetRotation.Yaw + RotationAmount * (DeltaTime / (1.0f / 30.0f)));
			SetRootCollisionRotation(TargetRotation);
		}
		TargetRotation = GetRootCollisionRotation();
	}
}

void UStalkerCharacterMovementComponent::UpdateAirborneRotation(float DeltaTime)
{
	if (RotationMode.VelocityDirection() || RotationMode.LookingDirection())
	{
		RotateRootCollision({0.0f, TargetRotation.Yaw, 0.0f}, 0.0f, 2.5f, DeltaTime);
		TargetRotation = GetSpeedXY() > 100.0 ? VelocityRotation : GetRootCollisionRotation();
	}
	else if (RotationMode == ECharacterRotationMode:: ControlDirection)
	{
		RotateRootCollision({0.0f, ViewRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		TargetRotation = GetRootCollisionRotation();
	}
}

void UStalkerCharacterMovementComponent::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
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

void UStalkerCharacterMovementComponent::RotateRootCollision(const FRotator& Target, float TargetInterpSpeed,
                                                    float ActorInterpSpeed, float DeltaTime)
{
	TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetRootCollisionRotation(FMath::RInterpTo(GetRootCollisionRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

bool UStalkerCharacterMovementComponent::CanSprint() const
{
	bool bCannotSprint = !IsMovingOnGround() || IsFalling() || bWantsToJump || bWantsToCrouch;
	if (!bCannotSprint)
	{
		if (!bHasMovementInput || RotationMode == ECharacterRotationMode::ControlDirection)
		{
			return false;
		}

		if (RotationMode == ECharacterRotationMode::VelocityDirection)
		{
			return MovementInputValue > 0.9f;
		}

		if (RotationMode == ECharacterRotationMode::LookingDirection)
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

bool UStalkerCharacterMovementComponent::CanCrouch() const
{
	bool bCannotCrouch = !CanEverCrouch() || !IsMovingOnGround() || IsFalling() || bWantsToJump || bWantsToSprint;
	return bCanCrouch & !bCannotCrouch;
}

bool UStalkerCharacterMovementComponent::CanJump() const
{
	bool bCannotJump = !CanEverJump() || !IsMovingOnGround() || bWantsToCrouch;
	return bCanJump & !bCannotJump;
}

void UStalkerCharacterMovementComponent::CalculateMovement(float DeltaSeconds)
{
	if (!IsSimulatedProxy())
	{
		PrevCharacterRotation = GetInRotation();
		ViewRotation = FMath::RInterpTo(ViewRotation, GetInControlRotation(), DeltaSeconds, ViewInterpSpeed);
	}
	
	PrevComponentRotation = GetRootCollisionRotation();
	
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

	if (CharMovementState.Grounded())
	{
		UpdateGait();
		UpdateGroundRotation(DeltaSeconds);
	}
	else if (CharMovementState.Airborne())
	{
		UpdateAirborneRotation(DeltaSeconds);
	}
	
	PreviousVelocity = GetVelocity();
	PreviousViewYaw = ViewRotation.Yaw;
	
	MaxDesiredSpeed = FMath::FInterpTo(MaxDesiredSpeed, TargetMaxSpeed, DeltaSeconds, SpeedInterpSpeed);
}

void UStalkerCharacterMovementComponent::Sprinting()
{
	if (bJustSprinting)
	{
		OnSprint(true);
	}
	else
	{
		OnSprint(false);
	}
}

void UStalkerCharacterMovementComponent::Crouching()
{
	if (bJustCrouched)
	{
		OnCrouch();
	}
	else
	{
		OnUnCrouch();
	}
}

void UStalkerCharacterMovementComponent::Jumping()
{
	if (bJustJumped)
	{
		OnJump();
	}
}

void UStalkerCharacterMovementComponent::OnSprint(bool bEnabled)
{
	bEnabled ? SetInputGait(ECharacterGaitType::Sprint) : SetInputGait(ECharacterGaitType::Run);
}

void UStalkerCharacterMovementComponent::OnCrouch()
{
	bJustCrouched = true;
	SetStance(ECharacterStanceType::Crouching);
}

void UStalkerCharacterMovementComponent::OnUnCrouch()
{
	bJustCrouched = false;
	SetStance(ECharacterStanceType::Standing);
}

void UStalkerCharacterMovementComponent::OnJump()
{
	AddImpulse({0.0f, 0.0f, JumpForce}, true);
	
	if (CharMovementState == ECharacterMovementState::Ground)
	{
		if (Stance == ECharacterStanceType::Standing)
		{
			OnJumpedDelegate.Broadcast();
		}
		else if (Stance == ECharacterStanceType::Crouching)
		{
			OnUnCrouch();
		}
	}
}

void UStalkerCharacterMovementComponent::UpdateSprint(bool bRequestedSprint)
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

void UStalkerCharacterMovementComponent::UpdateCrouch(bool bRequestedCrouch)
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

void UStalkerCharacterMovementComponent::UpdateJump(bool bRequestedJump)
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
