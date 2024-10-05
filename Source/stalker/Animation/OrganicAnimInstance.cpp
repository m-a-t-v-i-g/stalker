// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#include "Animation/OrganicAnimInstance.h"
#include "Organic/BaseOrganic.h"
#include "Curves/CurveVector.h"
#include "Components/CapsuleComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"

void UOrganicAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OrganicPawn = Cast<ABaseOrganic>(TryGetPawnOwner());
	if (OrganicPawn)
	{
		OrganicPawn->OnJumpedDelegate.AddUniqueDynamic(this, &UOrganicAnimInstance::OnJumped);

		OrganicMovement = OrganicPawn->GetComponentByClass<UOrganicMovementComponent>();
	}
}

void UOrganicAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OrganicPawn || !OrganicMovement.IsValid()) return;
	
	UpdateMovementInfo(DeltaSeconds);

	if (MovementState.Grounded())
	{
		UpdateGroundedValues(DeltaSeconds);
	}
	else if (MovementState.Airborne())
	{
		UpdateAirborneValues(DeltaSeconds);
	}
	else if (MovementState.Ragdoll())
	{
		UpdateRagdollValues();
	}
}

void UOrganicAnimInstance::UpdateMovementInfo(float DeltaSeconds)
{
	Movement.Acceleration = OrganicMovement->GetInstantAcceleration();
	Movement.Velocity = OrganicMovement->GetVelocity();
	Movement.Speed = OrganicMovement->GetSpeedXY();
	
	Movement.ActorRotation = OrganicMovement->GetRootCollisionRotation();
	Movement.ViewRotation = OrganicMovement->GetInputRotation();
	Movement.ViewYawRate = OrganicMovement->GetViewYawRate();

	Movement.MovementInputAmount = OrganicMovement->GetMovementInputValue();
	Movement.bHasMovementInput = OrganicMovement->HasMovementInput();
	Movement.bIsMoving = OrganicMovement->IsMoving();

	Movement.PrevMovementState = OrganicMovement->GetPrevMovementState();

	MovementState = OrganicMovement->GetMovementState();
	RotationMode = OrganicMovement->GetRotationMode();
	Stance = OrganicMovement->GetStance();
	Gait = OrganicMovement->GetGait();

	UpdateViewValues(DeltaSeconds);
}

void UOrganicAnimInstance::UpdateViewValues(float DeltaSeconds)
{
}

void UOrganicAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	const FOrganicAnim_VelocityBlend& TargetBlend = CalculateVelocityBlend();
	VelocityBlend.Forward = FMath::FInterpTo(VelocityBlend.Forward, TargetBlend.Forward, DeltaSeconds,
	                                         Config.VelocityBlendInterpSpeed);
	VelocityBlend.Backward = FMath::FInterpTo(VelocityBlend.Backward, TargetBlend.Backward, DeltaSeconds,
	                                          Config.VelocityBlendInterpSpeed);
	VelocityBlend.Left = FMath::FInterpTo(VelocityBlend.Left, TargetBlend.Left, DeltaSeconds,
	                                      Config.VelocityBlendInterpSpeed);
	VelocityBlend.Right = FMath::FInterpTo(VelocityBlend.Right, TargetBlend.Right, DeltaSeconds,
	                                       Config.VelocityBlendInterpSpeed);

	Grounded.DiagonalScaleAmount = CalculateDiagonalScaleAmount();

	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, RelativeAccelerationAmount.Y, DeltaSeconds,
	                                 Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, RelativeAccelerationAmount.X, DeltaSeconds,
	                                 Config.GroundedLeanInterpSpeed);

	Grounded.WalkRunBlend = CalculateWalkRunBlend();
	Grounded.StrideBlend = CalculateStrideBlend();

	Grounded.StandingPlayRate = CalculateStandingPlayRate();
	Grounded.CrouchingPlayRate = CalculateCrouchingPlayRate();
}

void UOrganicAnimInstance::UpdateRotationValues()
{
	MovementDirection = CalculateMovementDirection();

	FRotator Delta = Movement.Velocity.ToOrientationRotator() - Movement.ViewRotation;
	Delta.Normalize();
	
	const FVector& FBOffset = YawOffset_FB->GetVectorValue(Delta.Yaw);
	Grounded.FYaw = FBOffset.X;
	Grounded.BYaw = FBOffset.Y;
	
	const FVector& LROffset = YawOffset_LR->GetVectorValue(Delta.Yaw);
	Grounded.LYaw = LROffset.X;
	Grounded.RYaw = LROffset.Y;
}

void UOrganicAnimInstance::UpdateGroundedValues(float DeltaSeconds)
{
	bool bPreviousShouldMove = Grounded.bShouldMove;
	Grounded.bShouldMove = ShouldMoveCheck();

	if (!bPreviousShouldMove && Grounded.bShouldMove)
	{
		TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		Grounded.bRotateL = false;
		Grounded.bRotateR = false;
	}

	if (Grounded.bShouldMove)
	{
		UpdateMovementValues(DeltaSeconds);
		UpdateRotationValues();
	}
	else
	{
		if (CanRotateInPlace())
		{
			RotateInPlaceCheck();
		}
		else
		{
			Grounded.bRotateL = false;
			Grounded.bRotateR = false;
		}

		if (CanTurnInPlace())
		{
			TurnInPlaceCheck(DeltaSeconds);
		}
		else
		{
			TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		}
	}
}

void UOrganicAnimInstance::UpdateAirborneValues(float DeltaSeconds)
{
	Airborne.FallSpeed = Movement.Velocity.Z;
	Airborne.LandPrediction = CalculateLandPrediction();

	const FOrganicAnim_LeanAmount& AirborneLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, AirborneLeanAmount.LR, DeltaSeconds, Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, AirborneLeanAmount.FB, DeltaSeconds, Config.GroundedLeanInterpSpeed);
}

void UOrganicAnimInstance::UpdateRagdollValues()
{
	float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(NAME__ALSCharacterAnimInstance__root).Size();
	FlailRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

void UOrganicAnimInstance::SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve,
                                          FName IKFootBone, float& CurFootLockAlpha, bool& UseFootLockCurve,
                                          FVector& CurFootLockLoc, FRotator& CurFootLockRot)
{
}

void UOrganicAnimInstance::SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot)
{
}

void UOrganicAnimInstance::SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget)
{
}

void UOrganicAnimInstance::ResetIKOffsets(float DeltaSeconds)
{
}

void UOrganicAnimInstance::SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone,
                                          FName RootBone, FVector& CurLocationTarget, FVector& CurLocationOffset,
                                          FRotator& CurRotationOffset)
{
}

void UOrganicAnimInstance::RotateInPlaceCheck()
{
	Grounded.bRotateL = View.AimingAngle.X < RotateInPlace.RotateMinThreshold;
	Grounded.bRotateR = View.AimingAngle.X > RotateInPlace.RotateMaxThreshold;

	if (Grounded.bRotateL || Grounded.bRotateR)
	{
		Grounded.RotateRate = FMath::GetMappedRangeValueClamped<float, float>(
			{RotateInPlace.ViewYawRateMinRange, RotateInPlace.ViewYawRateMaxRange},
			{RotateInPlace.MinPlayRate, RotateInPlace.MaxPlayRate}, Movement.ViewYawRate);
	}
}

void UOrganicAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
{
	if (FMath::Abs(View.AimingAngle.X) <= TurnInPlaceValues.TurnCheckMinAngle ||
		Movement.ViewYawRate >= TurnInPlaceValues.AimYawRateLimit)
	{
		TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		return;
	}

	TurnInPlaceValues.ElapsedDelayTime += DeltaSeconds;
	float ClampedAimAngle = FMath::GetMappedRangeValueClamped<float, float>(
		{TurnInPlaceValues.TurnCheckMinAngle, 180.0f},
		{TurnInPlaceValues.MinAngleDelay, TurnInPlaceValues.MaxAngleDelay}, View.AimingAngle.X);

	if (TurnInPlaceValues.ElapsedDelayTime > ClampedAimAngle)
	{
		FRotator TurnInPlaceYawRot = Movement.ViewRotation;
		TurnInPlaceYawRot.Roll = 0.0f;
		TurnInPlaceYawRot.Pitch = 0.0f;

		TurnInPlace(TurnInPlaceYawRot, 1.0f, 0.0f, false);
	}
}

void UOrganicAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime,
									   bool OverrideCurrent)
{
	FRotator Delta = TargetRotation - Movement.ActorRotation;
	Delta.Normalize();

	float TurnAngle = Delta.Yaw;
	FAnimationSet_TurnInPlaceAsset TargetTurnAsset;

	if (Stance.Standing())
	{
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.N_TurnIP_L90
								  : TurnInPlaceValues.N_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.N_TurnIP_L180
								  : TurnInPlaceValues.N_TurnIP_R180;
		}
	}
	else
	{
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.CLF_TurnIP_L90
								  : TurnInPlaceValues.CLF_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? TurnInPlaceValues.CLF_TurnIP_L180
								  : TurnInPlaceValues.CLF_TurnIP_R180;
		}
	}

	if (!OverrideCurrent && IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		return;
	}
	
	PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName, 0.2f, 0.2f,
									  TargetTurnAsset.PlayRate * PlayRateScale, 1, 0.0f, StartTime);

	if (TargetTurnAsset.ScaleTurnAngle)
	{
		Grounded.RotationScale = (TurnAngle / TargetTurnAsset.AnimatedAngle) * TargetTurnAsset.PlayRate * PlayRateScale;
	}
	else
	{
		Grounded.RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
	}
}

float UOrganicAnimInstance::CalculateStrideBlend() const
{
	const float CurveTime = Movement.Speed / GetOwningComponent()->GetComponentScale().Z;
	const float ClampedGait = GetAnimCurveClamped(NAME_W_Gait, -1.0, 0.0f, 1.0f);
	const float LerpStrideBlend = FMath::Lerp(StrideBlend_N_Walk->GetFloatValue(CurveTime),
	                                          StrideBlend_N_Run->GetFloatValue(CurveTime), ClampedGait);
	
	return FMath::Lerp(LerpStrideBlend, StrideBlend_C_Walk->GetFloatValue(Movement.Speed),
	                   GetCurveValue(NAME_BasePose_CLF));
}

float UOrganicAnimInstance::CalculateWalkRunBlend() const
{
	return Gait.Walking() ? 0.0f : 1.0;
}

float UOrganicAnimInstance::CalculateStandingPlayRate() const
{
	const float LerpSpeed = FMath::Lerp(Movement.Speed / Config.AnimatedWalkSpeed,
	                                    Movement.Speed / Config.AnimatedRunSpeed,
	                                    GetAnimCurveClamped(NAME_W_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpSpeed, Movement.Speed / Config.AnimatedSprintSpeed,
	                                              GetAnimCurveClamped(NAME_W_Gait, -2.0f, 0.0f, 1.0f));

	return FMath::Clamp((SprintAffectedSpeed / Grounded.StrideBlend) / GetOwningComponent()->GetComponentScale().Z,
	                    0.0f, 3.0f);
}

float UOrganicAnimInstance::CalculateDiagonalScaleAmount() const
{
	return DiagonalScaleAmountCurve->GetFloatValue(FMath::Abs(VelocityBlend.Forward + VelocityBlend.Backward));
}

float UOrganicAnimInstance::CalculateCrouchingPlayRate() const
{
	return FMath::Clamp(
		Movement.Speed / Config.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
		GetComponentScale().Z,
		0.0f, 2.0f);
}

float UOrganicAnimInstance::CalculateLandPrediction() const
{
	if (Airborne.FallSpeed >= -200.0f)
	{
		return 0.0f;
	}

	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(OrganicPawn->GetCapsuleComponent());
	check(CapsuleComp);

	const FVector& CapsuleWorldLoc = CapsuleComp->GetComponentLocation();
	const float VelocityZ = Movement.Velocity.Z;

	FVector VelocityClamped = Movement.Velocity;
	VelocityClamped.Z = FMath::Clamp(VelocityZ, -4000.0f, -200.0f);
	VelocityClamped.Normalize();

	const FVector TraceLength = VelocityClamped * FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, -4000.0f}, {50.0f, 2000.0f}, VelocityZ);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OrganicPawn);

	FHitResult HitResult;
	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(CapsuleComp->GetUnscaledCapsuleRadius(),
	                                                                           CapsuleComp->GetUnscaledCapsuleHalfHeight());

	GetWorld()->SweepSingleByChannel(HitResult, CapsuleWorldLoc, CapsuleWorldLoc + TraceLength, FQuat::Identity,
	                                 ECC_Visibility, CapsuleCollisionShape, Params);

	if (OrganicMovement->HitWalkableFloor(HitResult))
	{
		return FMath::Lerp(LandPredictionCurve->GetFloatValue(HitResult.Time), 0.0f,
		                   GetCurveValue(NAME_Mask_LandPrediction));
	}
	return 0.0f;
}

FVector UOrganicAnimInstance::CalculateRelativeAccelerationAmount() const
{
	if (FVector::DotProduct(Movement.Acceleration, Movement.Velocity) > 0.0f)
	{
		float MaxAcc = OrganicMovement->GetInputAcceleration();
		return Movement.ActorRotation.UnrotateVector(Movement.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
	}

	float MaxBrakingDec = OrganicMovement->GetBrakingDeceleration();
	return Movement.ActorRotation.UnrotateVector(Movement.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

FOrganicAnim_VelocityBlend UOrganicAnimInstance::CalculateVelocityBlend() const
{
	const FVector LocRelativeVelocityDir =
		Movement.ActorRotation.UnrotateVector(Movement.Velocity.GetSafeNormal(0.1f));
	
	const float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) +
		FMath::Abs(LocRelativeVelocityDir.Z);
	
	const FVector RelativeDir = LocRelativeVelocityDir / Sum;
	
	FOrganicAnim_VelocityBlend Result;
	Result.Forward = FMath::Clamp(RelativeDir.X, 0.0f, 1.0f);
	Result.Backward = FMath::Abs(FMath::Clamp(RelativeDir.X, -1.0f, 0.0f));
	Result.Left = FMath::Abs(FMath::Clamp(RelativeDir.Y, -1.0f, 0.0f));
	Result.Right = FMath::Clamp(RelativeDir.Y, 0.0f, 1.0f);
	
	return Result;
}

FOrganicAnim_LeanAmount UOrganicAnimInstance::CalculateAirLeanAmount() const
{
	FOrganicAnim_LeanAmount CalcLeanAmount;
	const FVector& UnrotatedVel = Movement.ActorRotation.UnrotateVector(Movement.Velocity) / 350.0f;
	FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
	InversedVect *= LeanInAirCurve->GetFloatValue(Airborne.FallSpeed);
	CalcLeanAmount.LR = InversedVect.X;
	CalcLeanAmount.FB = InversedVect.Y;
	return CalcLeanAmount;
}

EOrganicMovementDirection UOrganicAnimInstance::CalculateMovementDirection() const
{
	if (Gait.Sprinting() || RotationMode.VelocityDirection())
	{
		return EOrganicMovementDirection::Forward;
	}

	FRotator Delta = Movement.Velocity.ToOrientationRotator() - Movement.ViewRotation;
	Delta.Normalize();
	return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

float UOrganicAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

bool UOrganicAnimInstance::ShouldMoveCheck() const
{
	return (Movement.bIsMoving && Movement.bHasMovementInput) || Movement.Speed > 150.0f;
}

bool UOrganicAnimInstance::CanRotateInPlace() const
{
	return RotationMode.ControlDirection();
}

bool UOrganicAnimInstance::CanTurnInPlace() const
{
	return RotationMode.LookingDirection() && GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

void UOrganicAnimInstance::OnJumped()
{
	Airborne.bJumped = true;
	Airborne.JumpPlayRate = FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, 600.0f}, {1.2f, 1.5f}, Movement.Speed);

	GetWorld()->GetTimerManager().SetTimer(OnJumpedTimer, this, &UOrganicAnimInstance::OnJumpedDelay, 0.1f, false);
}

void UOrganicAnimInstance::OnPivot()
{
	Grounded.bPivot = Movement.Speed < Config.TriggerPivotSpeedLimit;

	GetWorld()->GetTimerManager().SetTimer(OnPivotTimer, this, &UOrganicAnimInstance::OnPivotDelay, 0.1f, false);
}

void UOrganicAnimInstance::OnJumpedDelay()
{
	Airborne.bJumped = false;
}

void UOrganicAnimInstance::OnPivotDelay()
{
	Grounded.bPivot = false;
}

bool UOrganicAnimInstance::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	if (IncreaseBuffer)
	{
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
}

EOrganicMovementDirection UOrganicAnimInstance::CalculateQuadrant(EOrganicMovementDirection Current, float FRThreshold,
                                                                  float FLThreshold, float BRThreshold,
                                                                  float BLThreshold, float Buffer, float Angle)
{
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer,
	                 Current != EOrganicMovementDirection::Forward && Current != EOrganicMovementDirection::Backward))
	{
		return EOrganicMovementDirection::Forward;
	}
	if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer,
	                 Current != EOrganicMovementDirection::Right && Current != EOrganicMovementDirection::Left))
	{
		return EOrganicMovementDirection::Right;
	}
	if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer,
	                 Current != EOrganicMovementDirection::Right && Current != EOrganicMovementDirection::Left))
	{
		return EOrganicMovementDirection::Left;
	}
	return EOrganicMovementDirection::Backward;
}
