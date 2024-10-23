// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#include "Animation/Components/CharacterLocomotionAnimComponent.h"
#include "Animation/AnimationCore.h"
#include "Animation/OrganicAnimConfig.h"
#include "Components/CapsuleComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"
#include "Curves/CurveVector.h"

void UCharacterLocomotionAnimComponent::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (TryGetPawnOwner())
	{
		OrganicMovement = TryGetPawnOwner()->GetComponentByClass<UOrganicMovementComponent>();

		if (OrganicMovement.IsValid())
		{
			CollisionShape = MakeCollisionShape();
			OrganicMovement->OnJumpedDelegate.AddUniqueDynamic(this, &UCharacterLocomotionAnimComponent::OnJumped);
		}
	}
}

void UCharacterLocomotionAnimComponent::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OrganicMovement.IsValid()) return;
	
	UpdateMovementInfo(DeltaSeconds);
	UpdateViewInfo(DeltaSeconds);

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

void UCharacterLocomotionAnimComponent::UpdateMovementInfo(float DeltaSeconds)
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
}

void UCharacterLocomotionAnimComponent::UpdateViewInfo(float DeltaSeconds)
{
	View.SmoothedViewRotation = FMath::RInterpTo(View.SmoothedViewRotation, Movement.ViewRotation, DeltaSeconds,
												 AnimConfig->OrganicConfig.SmoothedAimingRotationInterpSpeed);

	FRotator Delta = Movement.ViewRotation - Movement.ActorRotation;
	Delta.Normalize();
	
	View.AimingAngle.X = Delta.Yaw;
	View.AimingAngle.Y = Delta.Pitch;

	Delta = View.SmoothedViewRotation - Movement.ActorRotation;
	Delta.Normalize();
	
	OrganicAnimData.SmoothedAimingAngle.X = Delta.Yaw;
	OrganicAnimData.SmoothedAimingAngle.Y = Delta.Pitch;

	if (!RotationMode.VelocityDirection())
	{
		View.AimSweepTime = FMath::GetMappedRangeValueClamped<float, float>({-90.0f, 90.0f}, {1.0f, 0.0f}, View.AimingAngle.Y);
		View.SpineRotation.Roll = 0.0f;
		View.SpineRotation.Pitch = 0.0f;
		View.SpineRotation.Yaw = View.AimingAngle.X / 4.0f;
	}
	else if (Movement.bHasMovementInput)
	{
		Delta = Movement.Acceleration.ToOrientationRotator() - Movement.ActorRotation;
		Delta.Normalize();
		
		const float InterpTarget = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f}, Delta.Yaw);
		View.InputYawOffsetTime = FMath::FInterpTo(View.InputYawOffsetTime, InterpTarget, DeltaSeconds, AnimConfig->OrganicConfig.InputYawOffsetInterpSpeed);
	}

	View.LeftYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 0.0f}, FMath::Abs(OrganicAnimData.SmoothedAimingAngle.X));
	View.RightYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 1.0f}, FMath::Abs(OrganicAnimData.SmoothedAimingAngle.X));
	View.ForwardYawTime = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f}, OrganicAnimData.SmoothedAimingAngle.X);
}

void UCharacterLocomotionAnimComponent::UpdateMovementValues(float DeltaSeconds)
{
	const FOrganicAnim_VelocityBlend& TargetBlend = CalculateVelocityBlend();
	
	VelocityBlend.Forward = FMath::FInterpTo(VelocityBlend.Forward, TargetBlend.Forward, DeltaSeconds,
	                                         AnimConfig->OrganicConfig.VelocityBlendInterpSpeed);
	VelocityBlend.Backward = FMath::FInterpTo(VelocityBlend.Backward, TargetBlend.Backward, DeltaSeconds,
	                                          AnimConfig->OrganicConfig.VelocityBlendInterpSpeed);
	VelocityBlend.Left = FMath::FInterpTo(VelocityBlend.Left, TargetBlend.Left, DeltaSeconds,
	                                      AnimConfig->OrganicConfig.VelocityBlendInterpSpeed);
	VelocityBlend.Right = FMath::FInterpTo(VelocityBlend.Right, TargetBlend.Right, DeltaSeconds,
	                                       AnimConfig->OrganicConfig.VelocityBlendInterpSpeed);

	Grounded.DiagonalScale = CalculateDiagonalScale();

	OrganicAnimData.RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, OrganicAnimData.RelativeAccelerationAmount.Y, DeltaSeconds,
	                                 AnimConfig->OrganicConfig.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, OrganicAnimData.RelativeAccelerationAmount.X, DeltaSeconds,
	                                 AnimConfig->OrganicConfig.GroundedLeanInterpSpeed);

	Grounded.WalkRunBlend = CalculateGaitBlend();
	Grounded.StrideBlend = CalculateStrideBlend();

	Grounded.StandingPlayRate = CalculateStandingPlayRate();
	Grounded.CrouchingPlayRate = CalculateCrouchingPlayRate();
}

void UCharacterLocomotionAnimComponent::UpdateRotationValues()
{
	MovementDirection = CalculateMovementDirection();

	FRotator Delta = Movement.Velocity.ToOrientationRotator() - Movement.ViewRotation;
	Delta.Normalize();
	
	const FVector& FBOffset = AnimConfig->YawOffset_FB->GetVectorValue(Delta.Yaw);
	Grounded.FYaw = FBOffset.X;
	Grounded.BYaw = FBOffset.Y;
	
	const FVector& LROffset = AnimConfig->YawOffset_LR->GetVectorValue(Delta.Yaw);
	Grounded.LYaw = LROffset.X;
	Grounded.RYaw = LROffset.Y;
}

void UCharacterLocomotionAnimComponent::UpdateGroundedValues(float DeltaSeconds)
{
	bool bPreviousShouldMove = Grounded.bShouldMove;
	Grounded.bShouldMove = ShouldMoveCheck();

	if (!bPreviousShouldMove && Grounded.bShouldMove)
	{
		OrganicAnimData.ElapsedDelayTime = 0.0f;
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
			OrganicAnimData.ElapsedDelayTime = 0.0f;
		}
	}
}

void UCharacterLocomotionAnimComponent::UpdateAirborneValues(float DeltaSeconds)
{
	Airborne.FallSpeed = Movement.Velocity.Z;
	Airborne.LandPrediction = CalculateLandPrediction();

	const FOrganicAnim_LeanAmount& AirborneLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, AirborneLeanAmount.LR, DeltaSeconds, AnimConfig->OrganicConfig.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, AirborneLeanAmount.FB, DeltaSeconds, AnimConfig->OrganicConfig.GroundedLeanInterpSpeed);
}

void UCharacterLocomotionAnimComponent::UpdateRagdollValues()
{
	float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(FOrganicBoneName::NAME_Root).Size();
	OrganicAnimData.FlailRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

void UCharacterLocomotionAnimComponent::RotateInPlaceCheck()
{
	Grounded.bRotateL = View.AimingAngle.X < AnimConfig->RotateInPlace.RotateMinThreshold;
	Grounded.bRotateR = View.AimingAngle.X > AnimConfig->RotateInPlace.RotateMaxThreshold;

	if (Grounded.bRotateL || Grounded.bRotateR)
	{
		Grounded.RotateRate = FMath::GetMappedRangeValueClamped<float, float>(
			{AnimConfig->RotateInPlace.ViewYawRateMinRange, AnimConfig->RotateInPlace.ViewYawRateMaxRange},
			{AnimConfig->RotateInPlace.MinPlayRate, AnimConfig->RotateInPlace.MaxPlayRate}, Movement.ViewYawRate);
	}
}

void UCharacterLocomotionAnimComponent::TurnInPlaceCheck(float DeltaSeconds)
{
	if (FMath::Abs(View.AimingAngle.X) <= AnimConfig->TurnInPlace.TurnCheckMinAngle || Movement.ViewYawRate >=
		AnimConfig->TurnInPlace.AimYawRateLimit)
	{
		OrganicAnimData.ElapsedDelayTime = 0.0f;
		return;
	}

	OrganicAnimData.ElapsedDelayTime += DeltaSeconds;
	
	float ClampedAimAngle = FMath::GetMappedRangeValueClamped<float, float>(
		{AnimConfig->TurnInPlace.TurnCheckMinAngle, 180.0f},
		{AnimConfig->TurnInPlace.MinAngleDelay, AnimConfig->TurnInPlace.MaxAngleDelay}, View.AimingAngle.X);

	if (OrganicAnimData.ElapsedDelayTime > ClampedAimAngle)
	{
		FRotator TurnInPlaceYawRot = Movement.ViewRotation;
		TurnInPlaceYawRot.Roll = 0.0f;
		TurnInPlaceYawRot.Pitch = 0.0f;

		TurnInPlace(TurnInPlaceYawRot, 1.0f, 0.0f, false);
	}
}

void UCharacterLocomotionAnimComponent::TurnInPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime,
                                       bool OverrideCurrent)
{
	FRotator Delta = TargetRotation - Movement.ActorRotation;
	Delta.Normalize();

	float TurnAngle = Delta.Yaw;
	auto TargetTurnAsset = GetTurnInPlaceAsset(TurnAngle);

	if (!OverrideCurrent && IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		return;
	}
	
	PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName, 0.2f, 0.2f,
									  TargetTurnAsset.PlayRate * PlayRateScale, 1, 0.0f, StartTime);

	if (TargetTurnAsset.ScaleTurnAngle)
	{
		Grounded.RotationScale = TurnAngle / TargetTurnAsset.AnimatedAngle * TargetTurnAsset.PlayRate * PlayRateScale;
	}
	else
	{
		Grounded.RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
	}
}

FAnimConfig_TurnInPlaceAsset UCharacterLocomotionAnimComponent::GetTurnInPlaceAsset(float TurnAngle) const
{
	FAnimConfig_TurnInPlaceAsset TargetTurnAsset;
	if (Stance.Standing())
	{
		if (FMath::Abs(TurnAngle) < AnimConfig->TurnInPlace.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? AnimConfig->TurnInPlace.N_TurnIP_L90
								  : AnimConfig->TurnInPlace.N_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? AnimConfig->TurnInPlace.N_TurnIP_L180
								  : AnimConfig->TurnInPlace.N_TurnIP_R180;
		}
	}
	else
	{
		if (FMath::Abs(TurnAngle) < AnimConfig->TurnInPlace.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? AnimConfig->TurnInPlace.CLF_TurnIP_L90
								  : AnimConfig->TurnInPlace.CLF_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
								  ? AnimConfig->TurnInPlace.CLF_TurnIP_L180
								  : AnimConfig->TurnInPlace.CLF_TurnIP_R180;
		}
	}
	return TargetTurnAsset;
}

float UCharacterLocomotionAnimComponent::CalculateStrideBlend() const
{
	const float CurveTime = Movement.Speed / GetOwningComponent()->GetComponentScale().Z;
	const float ClampedGait = GetAnimCurveClamped(FOrganicCurveName::NAME_Gait, -1.0, 0.0f, 1.0f);
	const float LerpStrideBlend = FMath::Lerp(AnimConfig->StrideBlend_N_Walk->GetFloatValue(CurveTime),
	                                          AnimConfig->StrideBlend_N_Run->GetFloatValue(CurveTime), ClampedGait);
	
	return FMath::Lerp(LerpStrideBlend, AnimConfig->StrideBlend_C_Walk->GetFloatValue(Movement.Speed),
	                   GetCurveValue(FOrganicCurveName::NAME_BasePose_C));
}

float UCharacterLocomotionAnimComponent::CalculateGaitBlend() const
{
	return Gait.Walking() ? 0.0f : 1.0;
}

float UCharacterLocomotionAnimComponent::CalculateStandingPlayRate() const
{
	const float LerpSpeed = FMath::Lerp(Movement.Speed / AnimConfig->OrganicConfig.AnimatedWalkSpeed,
	                                    Movement.Speed / AnimConfig->OrganicConfig.AnimatedRunSpeed,
	                                    GetAnimCurveClamped(FOrganicCurveName::NAME_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpSpeed, Movement.Speed / AnimConfig->OrganicConfig.AnimatedSprintSpeed,
	                                              GetAnimCurveClamped(FOrganicCurveName::NAME_Gait, -2.0f, 0.0f, 1.0f));

	return FMath::Clamp(SprintAffectedSpeed / Grounded.StrideBlend / GetOwningComponent()->GetComponentScale().Z, 0.0f, 3.0f);
}

float UCharacterLocomotionAnimComponent::CalculateDiagonalScale() const
{
	return AnimConfig->DiagonalScale->GetFloatValue(FMath::Abs(VelocityBlend.Forward + VelocityBlend.Backward));
}

float UCharacterLocomotionAnimComponent::CalculateCrouchingPlayRate() const
{
	return FMath::Clamp(
		Movement.Speed / AnimConfig->OrganicConfig.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
		GetComponentScale().Z, 0.0f, 2.0f);
}

float UCharacterLocomotionAnimComponent::CalculateLandPrediction() const
{
	if (Airborne.FallSpeed >= -200.0f)
	{
		return 0.0f;
	}

	if (const UShapeComponent* ShapeComp = Cast<UShapeComponent>(OrganicMovement->GetGenPawnOwner()->GetRootComponent()))
	{
		const FVector& CapsuleWorldLoc = ShapeComp->GetComponentLocation();
		const float VelocityZ = Movement.Velocity.Z;

		FVector VelocityClamped = Movement.Velocity;
		VelocityClamped.Z = FMath::Clamp(VelocityZ, -4000.0f, -200.0f);
		VelocityClamped.Normalize();

		const FVector TraceLength = VelocityClamped * FMath::GetMappedRangeValueClamped<float, float>(
			{0.0f, -4000.0f}, {50.0f, 2000.0f}, VelocityZ);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OrganicMovement->GetOwner());

		FHitResult HitResult;
		
		GetWorld()->SweepSingleByChannel(HitResult, CapsuleWorldLoc, CapsuleWorldLoc + TraceLength, FQuat::Identity,
		                                 ECC_Visibility, CollisionShape, Params);

		if (OrganicMovement->HitWalkableFloor(HitResult))
		{
			return FMath::Lerp(AnimConfig->LandPrediction->GetFloatValue(HitResult.Time), 0.0f,
			                   GetCurveValue(FOrganicCurveName::NAME_Mask_LandPrediction));
		}
	}
	return 0.0f;
}

FVector UCharacterLocomotionAnimComponent::CalculateRelativeAccelerationAmount() const
{
	if (FVector::DotProduct(Movement.Acceleration, Movement.Velocity) > 0.0f)
	{
		float MaxAcc = OrganicMovement->GetInputAcceleration();
		return Movement.ActorRotation.UnrotateVector(Movement.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
	}

	float MaxBrakingDec = OrganicMovement->GetBrakingDeceleration();
	return Movement.ActorRotation.UnrotateVector(Movement.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

FOrganicAnim_VelocityBlend UCharacterLocomotionAnimComponent::CalculateVelocityBlend() const
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

FOrganicAnim_LeanAmount UCharacterLocomotionAnimComponent::CalculateAirLeanAmount() const
{
	FOrganicAnim_LeanAmount CalcLeanAmount;
	const FVector& UnrotatedVel = Movement.ActorRotation.UnrotateVector(Movement.Velocity) / 350.0f;
	FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
	InversedVect *= AnimConfig->LeanAirborne->GetFloatValue(Airborne.FallSpeed);
	CalcLeanAmount.LR = InversedVect.X;
	CalcLeanAmount.FB = InversedVect.Y;
	return CalcLeanAmount;
}

EOrganicMovementDirection UCharacterLocomotionAnimComponent::CalculateMovementDirection() const
{
	if (Gait.Sprinting() || RotationMode.VelocityDirection())
	{
		return EOrganicMovementDirection::Forward;
	}

	FRotator Delta = Movement.Velocity.ToOrientationRotator() - Movement.ViewRotation;
	Delta.Normalize();
	return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

float UCharacterLocomotionAnimComponent::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

bool UCharacterLocomotionAnimComponent::ShouldMoveCheck() const
{
	return (Movement.bIsMoving && Movement.bHasMovementInput) || Movement.Speed > 150.0f;
}

bool UCharacterLocomotionAnimComponent::CanRotateInPlace() const
{
	return RotationMode.ControlDirection();
}

bool UCharacterLocomotionAnimComponent::CanTurnInPlace() const
{
	return RotationMode.LookingDirection() && GetCurveValue(FOrganicCurveName::NAME_Enable_Transition) >= 0.99f;
}

void UCharacterLocomotionAnimComponent::OnJumped()
{
	Airborne.bJumped = true;
	Airborne.JumpPlayRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 600.0f}, {1.2f, 1.5f}, Movement.Speed);

	GetWorld()->GetTimerManager().SetTimer(OnJumpedTimer, this, &UCharacterLocomotionAnimComponent::OnJumpedDelay, 0.1f, false);
}

void UCharacterLocomotionAnimComponent::OnPivot()
{
	Grounded.bPivot = Movement.Speed < AnimConfig->OrganicConfig.TriggerPivotSpeedLimit;

	GetWorld()->GetTimerManager().SetTimer(OnPivotTimer, this, &UCharacterLocomotionAnimComponent::OnPivotDelay, 0.1f, false);
}

void UCharacterLocomotionAnimComponent::OnJumpedDelay()
{
	Airborne.bJumped = false;
}

void UCharacterLocomotionAnimComponent::OnPivotDelay()
{
	Grounded.bPivot = false;
}

FCollisionShape UCharacterLocomotionAnimComponent::MakeCollisionShape() const
{
	FCollisionShape NewShape;
	switch (OrganicMovement->GetRootCollisionShape())
	{
	case EGenCollisionShape::VerticalCapsule:
	case EGenCollisionShape::HorizontalCapsule:
		{
			NewShape = FCollisionShape::MakeCapsule(OrganicMovement->GetRootCollisionExtent());
			break;
		}
	case EGenCollisionShape::Sphere:
		{
			NewShape = FCollisionShape::MakeSphere(OrganicMovement->GetRootCollisionExtent().X);
			break;
		}
	case EGenCollisionShape::Box:
		{
			NewShape = FCollisionShape::MakeBox(OrganicMovement->GetRootCollisionExtent());
			break;
		}
	default: break;
	}
	return NewShape;
}

bool UCharacterLocomotionAnimComponent::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	if (IncreaseBuffer)
	{
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
}

EOrganicMovementDirection UCharacterLocomotionAnimComponent::CalculateQuadrant(EOrganicMovementDirection Current, float FRThreshold,
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
