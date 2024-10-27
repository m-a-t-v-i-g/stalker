// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CharacterAnimInstance.h"
#include "AnimationCore.h"
#include "CharacterAnimConfig.h"
#include "Components/ShapeComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"
#include "Curves/CurveVector.h"
#include "Organic/Characters/BaseCharacter.h"

bool UCharacterAnimInstance::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	if (IncreaseBuffer)
	{
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
}

EOrganicMovementDirection UCharacterAnimInstance::CalculateQuadrant(EOrganicMovementDirection Current,
                                                                    float FRThreshold, float FLThreshold,
                                                                    float BRThreshold, float BLThreshold, float Buffer,
                                                                    float Angle)
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

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABaseCharacter>(TryGetPawnOwner());

	if (Character.IsValid())
	{
		OrganicMovement = Character->GetOrganicMovement();
		
		if (OrganicMovement.IsValid())
		{
			CollisionShape = MakeCollisionShape();
			OrganicMovement->OnJumpedDelegate.AddUniqueDynamic(this, &UCharacterAnimInstance::OnJumped);
		}
	}
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!Character.IsValid() || !OrganicMovement.IsValid()) return;
	
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
		UpdateRagdollValues(DeltaSeconds);
	}
}

void UCharacterAnimInstance::UpdateMovementInfo(float DeltaSeconds)
{
	if (!Character.IsValid()) return;
	
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
	MovementAction = Character->GetMovementAction();
	OverlayState = Character->GetOverlayState();
	
	// LayerBlendingValues.OverlayOverrideState = Character->GetOverlayOverrideState();
	
	UpdateLayerValues(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
}

void UCharacterAnimInstance::UpdateViewInfo(float DeltaSeconds)
{
	FRotator Delta = Movement.ViewRotation - Movement.ActorRotation;
	Delta.Normalize();

	View.AimingAngle.X = Delta.Yaw;
	View.AimingAngle.Y = Delta.Pitch;

	View.SmoothedViewRotation = FMath::RInterpTo(View.SmoothedViewRotation, Movement.ViewRotation, DeltaSeconds,
												 AnimConfig->CharacterConfig.SmoothedAimingRotationInterpSpeed);

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
		View.InputYawOffsetTime = FMath::FInterpTo(View.InputYawOffsetTime, InterpTarget, DeltaSeconds, AnimConfig->CharacterConfig.InputYawOffsetInterpSpeed);
	}

	View.LeftYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 0.0f}, FMath::Abs(OrganicAnimData.SmoothedAimingAngle.X));
	View.RightYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 1.0f}, FMath::Abs(OrganicAnimData.SmoothedAimingAngle.X));
	View.ForwardYawTime = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f}, OrganicAnimData.SmoothedAimingAngle.X);
}

void UCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	const FOrganicAnim_VelocityBlend& TargetBlend = CalculateVelocityBlend();
	
	VelocityBlend.Forward = FMath::FInterpTo(VelocityBlend.Forward, TargetBlend.Forward, DeltaSeconds,
											 AnimConfig->CharacterConfig.VelocityBlendInterpSpeed);
	VelocityBlend.Backward = FMath::FInterpTo(VelocityBlend.Backward, TargetBlend.Backward, DeltaSeconds,
											  AnimConfig->CharacterConfig.VelocityBlendInterpSpeed);
	VelocityBlend.Left = FMath::FInterpTo(VelocityBlend.Left, TargetBlend.Left, DeltaSeconds,
										  AnimConfig->CharacterConfig.VelocityBlendInterpSpeed);
	VelocityBlend.Right = FMath::FInterpTo(VelocityBlend.Right, TargetBlend.Right, DeltaSeconds,
										   AnimConfig->CharacterConfig.VelocityBlendInterpSpeed);

	Grounded.DiagonalScale = CalculateDiagonalScale();

	OrganicAnimData.RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, OrganicAnimData.RelativeAccelerationAmount.Y, DeltaSeconds,
									 AnimConfig->CharacterConfig.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, OrganicAnimData.RelativeAccelerationAmount.X, DeltaSeconds,
									 AnimConfig->CharacterConfig.GroundedLeanInterpSpeed);

	Grounded.WalkRunBlend = CalculateGaitBlend();
	Grounded.StrideBlend = CalculateStrideBlend();

	Grounded.StandingPlayRate = CalculateStandingPlayRate();
	Grounded.CrouchingPlayRate = CalculateCrouchingPlayRate();
}

void UCharacterAnimInstance::UpdateRotationValues(float DeltaSeconds)
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

void UCharacterAnimInstance::UpdateLayerValues(float DeltaSeconds)
{
	LayerBlending.EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(FCharacterCurveName::NAME_Mask_AimOffset));

	LayerBlending.BasePose_N = GetCurveValue(FCharacterCurveName::NAME_BasePose_N);
	LayerBlending.BasePose_CLF = GetCurveValue(FCharacterCurveName::NAME_BasePose_C);

	LayerBlending.Spine_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Spine_Add);
	LayerBlending.Head_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Head_Add);
	LayerBlending.Arm_L_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_L_Add);
	LayerBlending.Arm_R_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_R_Add);
	LayerBlending.Hand_L = GetCurveValue(FCharacterLayerName::NAME_Layering_Hand_L);
	LayerBlending.Hand_R = GetCurveValue(FCharacterLayerName::NAME_Layering_Hand_R);

	LayerBlending.EnableHandIK_L = FMath::Lerp(0.0f, GetCurveValue(FCharacterCurveName::NAME_Enable_HandIK_L),
													 GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_L));
	LayerBlending.EnableHandIK_R = FMath::Lerp(0.0f, GetCurveValue(FCharacterCurveName::NAME_Enable_HandIK_R),
													 GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_R));

	LayerBlending.Arm_L_LS = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_L_LS);
	LayerBlending.Arm_L_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlending.Arm_L_LS));
	LayerBlending.Arm_R_LS = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_R_LS);
	LayerBlending.Arm_R_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlending.Arm_R_LS));
}

void UCharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	SetFootLocking(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_L, FCharacterCurveName::NAME_FootLock_L,
				   FCharacterBoneName::NAME_IkFoot_L, FootIK.FootLock_L_Alpha, FootIK.UseFootLockCurve_L,
				   FootIK.FootLock_L_Location, FootIK.FootLock_L_Rotation);
	SetFootLocking(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_R, FCharacterCurveName::NAME_FootLock_R,
				   FCharacterBoneName::NAME_IkFoot_R, FootIK.FootLock_R_Alpha, FootIK.UseFootLockCurve_R,
				   FootIK.FootLock_R_Location, FootIK.FootLock_R_Rotation);

	if (MovementState.Airborne())
	{
		SetPelvisIKOffset(DeltaSeconds, FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets(DeltaSeconds);
	}
	else if (!MovementState.Ragdoll())
	{
		SetFootOffsets(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_L, FCharacterBoneName::NAME_IkFoot_L,
		               FCharacterBoneName::NAME_Root, FootOffsetLTarget,
		               FootIK.FootOffset_L_Location, FootIK.FootOffset_L_Rotation);
		SetFootOffsets(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_R, FCharacterBoneName::NAME_IkFoot_R,
		               FCharacterBoneName::NAME_Root, FootOffsetRTarget,
		               FootIK.FootOffset_R_Location, FootIK.FootOffset_R_Rotation);
		SetPelvisIKOffset(DeltaSeconds, FootOffsetLTarget, FootOffsetRTarget);
	}
}

void UCharacterAnimInstance::UpdateGroundedValues(float DeltaSeconds)
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
		UpdateRotationValues(DeltaSeconds);
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
		
		if (CanDynamicTransition())
		{
			DynamicTransitionCheck();
		}
	}
}

void UCharacterAnimInstance::UpdateAirborneValues(float DeltaSeconds)
{
	Airborne.FallSpeed = Movement.Velocity.Z;
	Airborne.LandPrediction = CalculateLandPrediction();

	const FOrganicAnim_LeanAmount& AirborneLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, AirborneLeanAmount.LR, DeltaSeconds, AnimConfig->CharacterConfig.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, AirborneLeanAmount.FB, DeltaSeconds, AnimConfig->CharacterConfig.GroundedLeanInterpSpeed);
}

void UCharacterAnimInstance::UpdateRagdollValues(float DeltaSeconds)
{
	float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(FCharacterBoneName::NAME_Root).Size();
	OrganicAnimData.FlailRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

void UCharacterAnimInstance::RotateInPlaceCheck()
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

void UCharacterAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
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

void UCharacterAnimInstance::TurnInPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent)
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

FAnimConfig_TurnInPlaceAsset UCharacterAnimInstance::GetTurnInPlaceAsset(float TurnAngle) const
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

float UCharacterAnimInstance::CalculateStrideBlend() const
{
	const float CurveTime = Movement.Speed / GetOwningComponent()->GetComponentScale().Z;
	const float ClampedGait = GetAnimCurveClamped(FCharacterCurveName::NAME_Gait, -1.0, 0.0f, 1.0f);
	const float LerpStrideBlend = FMath::Lerp(AnimConfig->StrideBlend_N_Walk->GetFloatValue(CurveTime),
											  AnimConfig->StrideBlend_N_Run->GetFloatValue(CurveTime), ClampedGait);
	
	return FMath::Lerp(LerpStrideBlend, AnimConfig->StrideBlend_C_Walk->GetFloatValue(Movement.Speed),
					   GetCurveValue(FCharacterCurveName::NAME_BasePose_C));
}

float UCharacterAnimInstance::CalculateGaitBlend() const
{
	return Gait.Walking() ? 0.0f : 1.0;
}

float UCharacterAnimInstance::CalculateStandingPlayRate() const
{
	const float LerpSpeed = FMath::Lerp(Movement.Speed / AnimConfig->CharacterConfig.AnimatedWalkSpeed,
										Movement.Speed / AnimConfig->CharacterConfig.AnimatedRunSpeed,
										GetAnimCurveClamped(FCharacterCurveName::NAME_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpSpeed, Movement.Speed / AnimConfig->CharacterConfig.AnimatedSprintSpeed,
												  GetAnimCurveClamped(FCharacterCurveName::NAME_Gait, -2.0f, 0.0f, 1.0f));

	return FMath::Clamp(SprintAffectedSpeed / Grounded.StrideBlend / GetOwningComponent()->GetComponentScale().Z, 0.0f, 3.0f);
}

float UCharacterAnimInstance::CalculateDiagonalScale() const
{
	return AnimConfig->DiagonalScale->GetFloatValue(FMath::Abs(VelocityBlend.Forward + VelocityBlend.Backward));
}

float UCharacterAnimInstance::CalculateCrouchingPlayRate() const
{
	return FMath::Clamp(
	Movement.Speed / AnimConfig->CharacterConfig.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
	GetComponentScale().Z, 0.0f, 2.0f);
}

float UCharacterAnimInstance::CalculateLandPrediction() const
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
							   GetCurveValue(FCharacterCurveName::NAME_Mask_LandPrediction));
		}
	}
	return 0.0f;
}

FVector UCharacterAnimInstance::CalculateRelativeAccelerationAmount() const
{
	if (FVector::DotProduct(Movement.Acceleration, Movement.Velocity) > 0.0f)
	{
		float MaxAcc = OrganicMovement->GetInputAcceleration();
		return Movement.ActorRotation.UnrotateVector(Movement.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
	}

	float MaxBrakingDec = OrganicMovement->GetBrakingDeceleration();
	return Movement.ActorRotation.UnrotateVector(Movement.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

FOrganicAnim_VelocityBlend UCharacterAnimInstance::CalculateVelocityBlend() const
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

FOrganicAnim_LeanAmount UCharacterAnimInstance::CalculateAirLeanAmount() const
{
	FOrganicAnim_LeanAmount CalcLeanAmount;
	const FVector& UnrotatedVel = Movement.ActorRotation.UnrotateVector(Movement.Velocity) / 350.0f;
	FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
	InversedVect *= AnimConfig->LeanAirborne->GetFloatValue(Airborne.FallSpeed);
	CalcLeanAmount.LR = InversedVect.X;
	CalcLeanAmount.FB = InversedVect.Y;
	return CalcLeanAmount;
}

EOrganicMovementDirection UCharacterAnimInstance::CalculateMovementDirection() const
{
	if (Gait.Sprinting() || RotationMode.VelocityDirection())
	{
		return EOrganicMovementDirection::Forward;
	}

	FRotator Delta = Movement.Velocity.ToOrientationRotator() - Movement.ViewRotation;
	Delta.Normalize();
	return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

float UCharacterAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

bool UCharacterAnimInstance::ShouldMoveCheck() const
{
	return (Movement.bIsMoving && Movement.bHasMovementInput) || Movement.Speed > 150.0f;
}

bool UCharacterAnimInstance::CanRotateInPlace() const
{
	return RotationMode.ControlDirection();
}

bool UCharacterAnimInstance::CanTurnInPlace() const
{
	return RotationMode.LookingDirection() && GetCurveValue(FCharacterCurveName::NAME_Enable_Transition) >= 0.99f;
}

void UCharacterAnimInstance::OnJumped()
{
	Airborne.bJumped = true;
	Airborne.JumpPlayRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 600.0f}, {1.2f, 1.5f}, Movement.Speed);

	GetWorld()->GetTimerManager().SetTimer(OnJumpedTimer, this, &UCharacterAnimInstance::OnJumpedDelay, 0.1f, false);
}

void UCharacterAnimInstance::OnPivot()
{
	Grounded.bPivot = Movement.Speed < AnimConfig->CharacterConfig.TriggerPivotSpeedLimit;

	GetWorld()->GetTimerManager().SetTimer(OnPivotTimer, this, &UCharacterAnimInstance::OnPivotDelay, 0.1f, false);
}

void UCharacterAnimInstance::OnJumpedDelay()
{
	Airborne.bJumped = false;
}

void UCharacterAnimInstance::OnPivotDelay()
{
	Grounded.bPivot = false;
}

void UCharacterAnimInstance::SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve,
                                            FName IKFootBone, float& CurFootLockAlpha, bool& UseFootLockCurve,
                                            FVector& CurFootLockLoc, FRotator& CurFootLockRot)
{
	if (GetCurveValue(EnableFootIKCurve) <= 0.0f)
	{
		return;
	}

	float FootLockCurveVal;
	if (UseFootLockCurve)
	{
		UseFootLockCurve = true;
		FootLockCurveVal = GetCurveValue(FootLockCurve) * (1.f / GetSkelMeshComponent()->AnimUpdateRateParams->UpdateRate);
	}
	else
	{
		UseFootLockCurve = true;
		FootLockCurveVal = 0.0f;
	}

	if (FootLockCurveVal >= 0.99f || FootLockCurveVal < CurFootLockAlpha)
	{
		CurFootLockAlpha = FootLockCurveVal;
	}

	if (CurFootLockAlpha >= 0.99f)
	{
		const FTransform& OwnerTransform = GetOwningComponent()->GetSocketTransform(IKFootBone, RTS_Component);
		CurFootLockLoc = OwnerTransform.GetLocation();
		CurFootLockRot = OwnerTransform.Rotator();
	}

	if (CurFootLockAlpha > 0.0f)
	{
		SetFootLockOffsets(DeltaSeconds, CurFootLockLoc, CurFootLockRot);
	}
}

void UCharacterAnimInstance::SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot)
{
	FRotator RotationDifference = FRotator::ZeroRotator;
	
	if (OrganicMovement->IsMovingOnGround())
	{
		if (RotationMode == EOrganicRotationMode::ControlDirection)
		{
			RotationDifference = OrganicMovement->GetRootCollisionRotation() - OrganicMovement->GetLastPawnRotation();
		}
		else if (RotationMode == EOrganicRotationMode::LookingDirection)
		{
			RotationDifference = OrganicMovement->GetRootCollisionRotation() - OrganicMovement->GetLastComponentRotation();
		}
		RotationDifference.Normalize();
	}

	const FVector& LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(Movement.Velocity * DeltaSeconds);
	LocalLoc = (LocalLoc - LocationDifference).RotateAngleAxis(RotationDifference.Yaw, FVector::DownVector);

	FRotator Delta = LocalRot - RotationDifference;
	Delta.Normalize();
	LocalRot = Delta;
}

void UCharacterAnimInstance::SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget)
{
	FootIK.PelvisAlpha = (GetCurveValue(FCharacterCurveName::NAME_Enable_FootIK_L) + GetCurveValue(
		FCharacterCurveName::NAME_Enable_FootIK_R)) / 2.0f;

	if (FootIK.PelvisAlpha > 0.0f)
	{
		FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;
		float InterpSpeed = PelvisTarget.Z > FootIK.PelvisOffset.Z ? 10.0f : 15.0f;

		FootIK.PelvisOffset = FMath::VInterpTo(FootIK.PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
	}
	else
	{
		FootIK.PelvisOffset = FVector::ZeroVector;
	}
}

void UCharacterAnimInstance::ResetIKOffsets(float DeltaSeconds)
{
	FootIK.FootOffset_L_Location = FMath::VInterpTo(FootIK.FootOffset_L_Location,
														  FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIK.FootOffset_R_Location = FMath::VInterpTo(FootIK.FootOffset_R_Location,
														  FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIK.FootOffset_L_Rotation = FMath::RInterpTo(FootIK.FootOffset_L_Rotation,
														  FRotator::ZeroRotator, DeltaSeconds, 15.0f);
	FootIK.FootOffset_R_Rotation = FMath::RInterpTo(FootIK.FootOffset_R_Rotation,
														  FRotator::ZeroRotator, DeltaSeconds, 15.0f);
}

void UCharacterAnimInstance::SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone,
                                            FName RootBone, FVector& CurLocationTarget, FVector& CurLocationOffset,
                                            FRotator& CurRotationOffset)
{
	if (GetCurveValue(EnableFootIKCurve) <= 0)
	{
		CurLocationOffset = FVector::ZeroVector;
		CurRotationOffset = FRotator::ZeroRotator;
		return;
	}

	USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	FVector IKFootFloorLoc = OwnerComp->GetSocketLocation(IKFootBone);
	IKFootFloorLoc.Z = OwnerComp->GetSocketLocation(RootBone).Z;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character.Get());

	const FVector TraceStart = IKFootFloorLoc + FVector(0.0, 0.0, GetAnimConfig<UCharacterAnimConfig>()->CharacterConfig.IK_TraceDistanceAboveFoot); // TODO
	const FVector TraceEnd = IKFootFloorLoc - FVector(0.0, 0.0, GetAnimConfig<UCharacterAnimConfig>()->CharacterConfig.IK_TraceDistanceBelowFoot); // TODO

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

	FRotator TargetRotOffset = FRotator::ZeroRotator;
	if (Character->GetOrganicMovement()->HitWalkableFloor(HitResult))
	{
		FVector ImpactPoint = HitResult.ImpactPoint;
		FVector ImpactNormal = HitResult.ImpactNormal;

		float FootHeight = GetAnimConfig<UCharacterAnimConfig>()->CharacterConfig.FootHeight; // TODO
		CurLocationTarget = ImpactPoint + ImpactNormal * FootHeight - (IKFootFloorLoc + FVector(0, 0, FootHeight));

		TargetRotOffset.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.X, ImpactNormal.Z));
		TargetRotOffset.Roll = FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z));
	}

	float InterpSpeed = CurLocationOffset.Z > CurLocationTarget.Z ? 30.f : 15.0f;
	CurLocationOffset = FMath::VInterpTo(CurLocationOffset, CurLocationTarget, DeltaSeconds, InterpSpeed);
	CurRotationOffset = FMath::RInterpTo(CurRotationOffset, TargetRotOffset, DeltaSeconds, 30.0f);
}

void UCharacterAnimInstance::DynamicTransitionCheck()
{
	FTransform SocketTransformA = GetOwningComponent()->GetSocketTransform(FCharacterBoneName::NAME_IkFoot_L, RTS_Component);
	FTransform SocketTransformB = GetOwningComponent()->GetSocketTransform(FCharacterBoneName::NAME_VB_Foot_Target_L, RTS_Component);

	float Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > GetAnimConfig<UCharacterAnimConfig>()->CharacterConfig.DynamicTransitionThreshold) // TODO
	{
		FOrganicDynamicMontage Params;
		Params.Animation = AnimConfig->TransitionAnim_R;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;

		PlayDynamicTransition(0.1f, Params);
	}

	SocketTransformA = GetOwningComponent()->GetSocketTransform(FCharacterBoneName::NAME_IkFoot_R, RTS_Component);
	SocketTransformB = GetOwningComponent()->GetSocketTransform(FCharacterBoneName::NAME_VB_Foot_Target_R, RTS_Component);

	Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > GetAnimConfig<UCharacterAnimConfig>()->CharacterConfig.DynamicTransitionThreshold) // TODO
	{
		FOrganicDynamicMontage Params;
		Params.Animation = AnimConfig->TransitionAnim_L;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;

		PlayDynamicTransition(0.1f, Params);
	}
}

void UCharacterAnimInstance::PlayTransition(const FOrganicDynamicMontage& Parameters)
{
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, FCharacterCurveName::NAME_Grounded_Slot,
									  Parameters.BlendInTime, Parameters.BlendOutTime, Parameters.PlayRate, 1,
									  0.0f, Parameters.StartTime);
}

void UCharacterAnimInstance::PlayTransitionChecked(const FOrganicDynamicMontage& Parameters)
{
	if (Stance.Standing() && !Grounded.bShouldMove)
	{
		PlayTransition(Parameters);
	}
}

void UCharacterAnimInstance::PlayDynamicTransition(float ReTriggerDelay, FOrganicDynamicMontage Parameters)
{
	if (bCanPlayDynamicTransition)
	{
		bCanPlayDynamicTransition = false;

		PlayTransition(Parameters);
		GetWorld()->GetTimerManager().SetTimer(PlayDynamicTransitionTimer, this,
		                                       &UCharacterAnimInstance::DynamicTransitionCheck, ReTriggerDelay,
		                                       false);
	}
}

void UCharacterAnimInstance::PlayDynamicTransitionDelay()
{
	bCanPlayDynamicTransition = true;
}

bool UCharacterAnimInstance::CanDynamicTransition() const
{
	return GetCurveValue(FCharacterCurveName::NAME_Enable_Transition) >= 0.99f;
}

FCollisionShape UCharacterAnimInstance::MakeCollisionShape() const
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

/*
// TODO:
if (auto Char = Character->FindComponentByClass<UCameraComponent>())
{
	FHitResult HitResult;
	FVector ViewLoc = Char->GetComponentLocation();
	FRotator ViewRot = Char->GetComponentRotation();
	FRotator LookTo;
	FVector PointToLook;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Char->GetComponentLocation(),
													 ViewLoc + ViewRot.Vector() * 10000.0f, ECC_Visibility);

	if (!bHit)
	{
		PointToLook = HitResult.TraceEnd;
	}
	else
	{
		PointToLook = HitResult.ImpactPoint;
	}

	FVector FromLoc = Character->GetActorLocation();
	FromLoc.Z = FromLoc.Z + 45.0f;
	LookTo = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::FindLookAtRotation(FromLoc, PointToLook), Movement.ActorRotation);

	View.AimOffset = FMath::Vector2DInterpTo(View.AimOffset, {LookTo.Yaw, LookTo.Pitch}, DeltaSeconds, 10.f);
}
*/
