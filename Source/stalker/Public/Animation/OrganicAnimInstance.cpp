// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#include "Animation/OrganicAnimInstance.h"
#include "GenCapsuleComponent.h"
#include "Organic/BaseOrganic.h"
#include "Curves/CurveVector.h"
#include "Components/CapsuleComponent.h"
#include "Components/Movement/OrganicMovementComponent.h"

static const FName NAME_BasePose_CLF(TEXT("BasePose_CLF"));
static const FName NAME_BasePose_N(TEXT("BasePose_N"));
static const FName NAME_Enable_FootIK_R(TEXT("Enable_FootIK_R"));
static const FName NAME_Enable_FootIK_L(TEXT("Enable_FootIK_L"));
static const FName NAME_Enable_HandIK_L(TEXT("Enable_HandIK_L"));
static const FName NAME_Enable_HandIK_R(TEXT("Enable_HandIK_R"));
static const FName NAME_Enable_Transition(TEXT("Enable_Transition"));
static const FName NAME_FootLock_L(TEXT("FootLock_L"));
static const FName NAME_FootLock_R(TEXT("FootLock_R"));
static const FName NAME_Grounded___Slot(TEXT("Grounded Slot"));
static const FName NAME_Layering_Arm_L(TEXT("Layering_Arm_L"));
static const FName NAME_Layering_Arm_L_Add(TEXT("Layering_Arm_L_Add"));
static const FName NAME_Layering_Arm_L_LS(TEXT("Layering_Arm_L_LS"));
static const FName NAME_Layering_Arm_R(TEXT("Layering_Arm_R"));
static const FName NAME_Layering_Arm_R_Add(TEXT("Layering_Arm_R_Add"));
static const FName NAME_Layering_Arm_R_LS(TEXT("Layering_Arm_R_LS"));
static const FName NAME_Layering_Hand_L(TEXT("Layering_Hand_L"));
static const FName NAME_Layering_Hand_R(TEXT("Layering_Hand_R"));
static const FName NAME_Layering_Head_Add(TEXT("Layering_Head_Add"));
static const FName NAME_Layering_Spine_Add(TEXT("Layering_Spine_Add"));
static const FName NAME_Mask_AimOffset(TEXT("Mask_AimOffset"));
static const FName NAME_Mask_LandPrediction(TEXT("Mask_LandPrediction"));
static const FName NAME__ALSCharacterAnimInstance__RotationAmount(TEXT("RotationAmount"));
static const FName NAME_VB___foot_target_l(TEXT("VB foot_target_l"));
static const FName NAME_VB___foot_target_r(TEXT("VB foot_target_r"));
static const FName NAME_W_Gait(TEXT("W_Gait"));
static const FName NAME__ALSCharacterAnimInstance__root(TEXT("root"));

void UOrganicAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OrganicPawn = Cast<ABaseOrganic>(TryGetPawnOwner());
	if (OrganicPawn)
	{
		//Character->OnJumpedDelegate.AddUniqueDynamic(this, &UOrganicAnimInstance::OnJumped);
	}
}

void UOrganicAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OrganicPawn) return;
	
	CharacterInformation.Rotation = OrganicPawn->GetOrganicMovement()->GetRootCollisionRotation();
	CharacterInformation.ViewRotation = OrganicPawn->GetOrganicViewRotation();
	CharacterInformation.AimYawRate = OrganicPawn->GetViewYawRate();
	
	CharacterInformation.Velocity = OrganicPawn->GetOrganicMovement()->GetVelocity();
	CharacterInformation.Acceleration = OrganicPawn->GetAcceleration();
	CharacterInformation.Speed = OrganicPawn->GetSpeed();
	
	CharacterInformation.bIsMoving = OrganicPawn->IsMoving();
	CharacterInformation.MovementInputAmount = OrganicPawn->GetMovementInputAmount();
	CharacterInformation.bHasMovementInput = OrganicPawn->HasMovementInput();
	
	CharacterInformation.PrevMovementState = OrganicPawn->GetPrevMovementState();
	
	MovementState = OrganicPawn->GetMovementState();
	RotationMode = OrganicPawn->GetRotationMode();
	Stance = OrganicPawn->GetStance();
	Gait = OrganicPawn->GetGait();
	
	UpdateAimingValues(DeltaSeconds);
	UpdateLayerValues();
	UpdateFootIK(DeltaSeconds);

	if (MovementState.Grounded())
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
			
			if (CanDynamicTransition())
			{
				DynamicTransitionCheck();
			}
		}
	}
	else if (MovementState.Airborne())
	{
		UpdateInAirValues(DeltaSeconds);
	}
	else if (MovementState.Ragdoll())
	{
		UpdateRagdollValues();
	}
}

void UOrganicAnimInstance::PlayTransition(const FOrganicDynamicMontage& Parameters)
{
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, NAME_Grounded___Slot,
	                                  Parameters.BlendInTime, Parameters.BlendOutTime, Parameters.PlayRate, 1,
	                                  0.0f, Parameters.StartTime);
}

void UOrganicAnimInstance::PlayTransitionChecked(const FOrganicDynamicMontage& Parameters)
{
	if (Stance.Standing() && !Grounded.bShouldMove)
	{
		PlayTransition(Parameters);
	}
}

void UOrganicAnimInstance::PlayDynamicTransition(float ReTriggerDelay, FOrganicDynamicMontage Parameters)
{
	if (bCanPlayDynamicTransition)
	{
		bCanPlayDynamicTransition = false;

		PlayTransition(Parameters);

		UWorld* World = GetWorld();
		check(World);
		World->GetTimerManager().SetTimer(PlayDynamicTransitionTimer, this,
		                                  &UOrganicAnimInstance::PlayDynamicTransitionDelay,
		                                  ReTriggerDelay, false);
	}
}

bool UOrganicAnimInstance::ShouldMoveCheck() const
{
	return (CharacterInformation.bIsMoving && CharacterInformation.bHasMovementInput) ||
		CharacterInformation.Speed > 150.0f;
}

bool UOrganicAnimInstance::CanRotateInPlace() const
{
	return RotationMode.ControlDirection();
}

bool UOrganicAnimInstance::CanTurnInPlace() const
{
	return RotationMode.LookingDirection() && GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

bool UOrganicAnimInstance::CanDynamicTransition() const
{
	return GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

void UOrganicAnimInstance::PlayDynamicTransitionDelay()
{
	bCanPlayDynamicTransition = true;
}

void UOrganicAnimInstance::OnJumpedDelay()
{
	InAir.bJumped = false;
}

void UOrganicAnimInstance::OnPivotDelay()
{
	Grounded.bPivot = false;
}

void UOrganicAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	AimingValues.SmoothedViewRotation = FMath::RInterpTo(AimingValues.SmoothedViewRotation,
	                                                       CharacterInformation.ViewRotation, DeltaSeconds,
	                                                       Config.SmoothedAimingRotationInterpSpeed);

	FRotator Delta = CharacterInformation.ViewRotation - CharacterInformation.Rotation;
	Delta.Normalize();
	AimingValues.AimingAngle.X = Delta.Yaw;
	AimingValues.AimingAngle.Y = Delta.Pitch;

	Delta = AimingValues.SmoothedViewRotation - CharacterInformation.Rotation;
	Delta.Normalize();
	SmoothedAimingAngle.X = Delta.Yaw;
	SmoothedAimingAngle.Y = Delta.Pitch;

	if (!RotationMode.VelocityDirection())
	{
		AimingValues.AimSweepTime = FMath::GetMappedRangeValueClamped<float, float>({-90.0f, 90.0f}, {1.0f, 0.0f},
		                                                              AimingValues.AimingAngle.Y);

		AimingValues.SpineRotation.Roll = 0.0f;
		AimingValues.SpineRotation.Pitch = 0.0f;
		AimingValues.SpineRotation.Yaw = AimingValues.AimingAngle.X / 4.0f;
	}
	else if (CharacterInformation.bHasMovementInput)
	{
		Delta = CharacterInformation.Acceleration.ToOrientationRotator() - CharacterInformation.Rotation;
		Delta.Normalize();
		const float InterpTarget = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f}, Delta.Yaw);

		AimingValues.InputYawOffsetTime = FMath::FInterpTo(AimingValues.InputYawOffsetTime, InterpTarget,
		                                                   DeltaSeconds, Config.InputYawOffsetInterpSpeed);
	}

	AimingValues.LeftYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 0.0f},
	                                                             FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.RightYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 1.0f},
	                                                              FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.ForwardYawTime = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f},
	                                                                SmoothedAimingAngle.X);
}

void UOrganicAnimInstance::UpdateLayerValues()
{
	
	LayerBlendingValues.EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(NAME_Mask_AimOffset));
	
	LayerBlendingValues.BasePose_N = GetCurveValue(NAME_BasePose_N);
	LayerBlendingValues.BasePose_CLF = GetCurveValue(NAME_BasePose_CLF);
	
	LayerBlendingValues.Spine_Add = GetCurveValue(NAME_Layering_Spine_Add);
	LayerBlendingValues.Head_Add = GetCurveValue(NAME_Layering_Head_Add);
	LayerBlendingValues.Arm_L_Add = GetCurveValue(NAME_Layering_Arm_L_Add);
	LayerBlendingValues.Arm_R_Add = GetCurveValue(NAME_Layering_Arm_R_Add);
	
	LayerBlendingValues.Hand_R = GetCurveValue(NAME_Layering_Hand_R);
	LayerBlendingValues.Hand_L = GetCurveValue(NAME_Layering_Hand_L);
	
	LayerBlendingValues.EnableHandIK_L = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_L),
	                                                 GetCurveValue(NAME_Layering_Arm_L));
	LayerBlendingValues.EnableHandIK_R = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_R),
	                                                 GetCurveValue(NAME_Layering_Arm_R));
	
	LayerBlendingValues.Arm_L_LS = GetCurveValue(NAME_Layering_Arm_L_LS);
	LayerBlendingValues.Arm_L_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_L_LS));
	LayerBlendingValues.Arm_R_LS = GetCurveValue(NAME_Layering_Arm_R_LS);
	LayerBlendingValues.Arm_R_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_R_LS));
}

void UOrganicAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_L, NAME_FootLock_L,
	               IkFootL_BoneName, FootIKValues.FootLock_L_Alpha, FootIKValues.UseFootLockCurve_L,
	               FootIKValues.FootLock_L_Location, FootIKValues.FootLock_L_Rotation);
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_R, NAME_FootLock_R,
	               IkFootR_BoneName, FootIKValues.FootLock_R_Alpha, FootIKValues.UseFootLockCurve_R,
	               FootIKValues.FootLock_R_Location, FootIKValues.FootLock_R_Rotation);

	if (MovementState.Airborne())
	{
		SetPelvisIKOffset(DeltaSeconds, FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets(DeltaSeconds);
	}
	else if (!MovementState.Ragdoll())
	{
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_L, IkFootL_BoneName, NAME__ALSCharacterAnimInstance__root,
		               FootOffsetLTarget,
		               FootIKValues.FootOffset_L_Location, FootIKValues.FootOffset_L_Rotation);
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_R, IkFootR_BoneName, NAME__ALSCharacterAnimInstance__root,
		               FootOffsetRTarget,
		               FootIKValues.FootOffset_R_Location, FootIKValues.FootOffset_R_Rotation);
		SetPelvisIKOffset(DeltaSeconds, FootOffsetLTarget, FootOffsetRTarget);
	}
}

void UOrganicAnimInstance::SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve,
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
		/*
		UseFootLockCurve = FMath::Abs(GetCurveValue(NAME__ALSCharacterAnimInstance__RotationAmount)) <= 0.001f ||
			Character->GetLocalRole() != ROLE_AutonomousProxy;
			*/
		
		FootLockCurveVal = GetCurveValue(FootLockCurve) * (1.f / GetSkelMeshComponent()->AnimUpdateRateParams->UpdateRate);
	}
	else
	{
		UseFootLockCurve = true;
		/*
		UseFootLockCurve = GetCurveValue(FootLockCurve) >= 0.99f;
		*/
		
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

void UOrganicAnimInstance::SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot)
{
	FRotator RotationDifference = FRotator::ZeroRotator;
	if (OrganicPawn->GetOrganicMovement()->IsMovingOnGround())
	{
		if (RotationMode == EOrganicRotationMode::ControlDirection)
		{
			RotationDifference = OrganicPawn->GetOrganicMovement()->GetRootCollisionRotation() - OrganicPawn->
				GetOrganicMovement()->GetLastPawnRotation();
		}
		else if (RotationMode == EOrganicRotationMode::LookingDirection)
		{
			RotationDifference = OrganicPawn->GetOrganicMovement()->GetRootCollisionRotation() - OrganicPawn->
				GetOrganicMovement()->GetLastComponentRotation();
		}
		RotationDifference.Normalize();
	}

	const FVector& LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(
		CharacterInformation.Velocity * DeltaSeconds);
	LocalLoc = (LocalLoc - LocationDifference).RotateAngleAxis(RotationDifference.Yaw, FVector::DownVector);

	FRotator Delta = LocalRot - RotationDifference;
	Delta.Normalize();
	LocalRot = Delta;
}

void UOrganicAnimInstance::SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget,
                                                  FVector FootOffsetRTarget)
{
	FootIKValues.PelvisAlpha =
		(GetCurveValue(NAME_Enable_FootIK_L) + GetCurveValue(NAME_Enable_FootIK_R)) / 2.0f;

	if (FootIKValues.PelvisAlpha > 0.0f)
	{
		const FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;
		const float InterpSpeed = PelvisTarget.Z > FootIKValues.PelvisOffset.Z ? 10.0f : 15.0f;
		FootIKValues.PelvisOffset =
			FMath::VInterpTo(FootIKValues.PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
	}
	else
	{
		FootIKValues.PelvisOffset = FVector::ZeroVector;
	}
}

void UOrganicAnimInstance::ResetIKOffsets(float DeltaSeconds)
{
	FootIKValues.FootOffset_L_Location = FMath::VInterpTo(FootIKValues.FootOffset_L_Location,
	                                                      FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Location = FMath::VInterpTo(FootIKValues.FootOffset_R_Location,
	                                                      FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_L_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_L_Rotation,
	                                                      FRotator::ZeroRotator, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_R_Rotation,
	                                                      FRotator::ZeroRotator, DeltaSeconds, 15.0f);
}

void UOrganicAnimInstance::SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone,
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

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OrganicPawn);

	const FVector TraceStart = IKFootFloorLoc + FVector(0.0, 0.0, Config.IK_TraceDistanceAboveFoot);
	const FVector TraceEnd = IKFootFloorLoc - FVector(0.0, 0.0, Config.IK_TraceDistanceBelowFoot);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult,
	                                                  TraceStart,
	                                                  TraceEnd,
	                                                  ECC_Visibility, Params);

	/*
	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugLineTraceSingle(
			World,
			TraceStart,
			TraceEnd,
			EDrawDebugTrace::Type::ForOneFrame,
			bHit,
			HitResult,
			FLinearColor::Red,
			FLinearColor::Green,
			5.0f);
	}
	*/

	FRotator TargetRotOffset = FRotator::ZeroRotator;
	if (OrganicPawn->GetOrganicMovement()->HitWalkableFloor(HitResult))
	{
		FVector ImpactPoint = HitResult.ImpactPoint;
		FVector ImpactNormal = HitResult.ImpactNormal;

		// Step 1.1: Find the difference in location from the Impact point and the expected (flat) floor location.
		// These values are offset by the normal multiplied by the
		// foot height to get better behavior on angled surfaces.
		CurLocationTarget = (ImpactPoint + ImpactNormal * Config.FootHeight) -
			(IKFootFloorLoc + FVector(0, 0, Config.FootHeight));

		// Step 1.2: Calculate the Rotation offset by getting the Atan2 of the Impact Normal.
		TargetRotOffset.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.X, ImpactNormal.Z));
		TargetRotOffset.Roll = FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z));
	}

	// Step 2: Interp the Current Location Offset to the new target value.
	// Interpolate at different speeds based on whether the new target is above or below the current one.
	const float InterpSpeed = CurLocationOffset.Z > CurLocationTarget.Z ? 30.f : 15.0f;
	CurLocationOffset = FMath::VInterpTo(CurLocationOffset, CurLocationTarget, DeltaSeconds, InterpSpeed);

	// Step 3: Interp the Current Rotation Offset to the new target value.
	CurRotationOffset = FMath::RInterpTo(CurRotationOffset, TargetRotOffset, DeltaSeconds, 30.0f);
}

void UOrganicAnimInstance::RotateInPlaceCheck()
{
	// Step 1: Check if the character should rotate left or right by checking if the Aiming Angle exceeds the threshold.
	Grounded.bRotateL = AimingValues.AimingAngle.X < RotateInPlace.RotateMinThreshold;
	Grounded.bRotateR = AimingValues.AimingAngle.X > RotateInPlace.RotateMaxThreshold;

	// Step 2: If the character should be rotating, set the Rotate Rate to scale with the Aim Yaw Rate.
	// This makes the character rotate faster when moving the camera faster.
	if (Grounded.bRotateL || Grounded.bRotateR)
	{
		Grounded.RotateRate = FMath::GetMappedRangeValueClamped<float, float>(
			{RotateInPlace.ViewYawRateMinRange, RotateInPlace.ViewYawRateMaxRange},
			{RotateInPlace.MinPlayRate, RotateInPlace.MaxPlayRate},
			CharacterInformation.AimYawRate);
	}
}

void UOrganicAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
{
	// Step 1: Check if Aiming angle is outside of the Turn Check Min Angle, and if the Aim Yaw Rate is below the Aim Yaw Rate Limit.
	// If so, begin counting the Elapsed Delay Time. If not, reset the Elapsed Delay Time.
	// This ensures the conditions remain true for a sustained period of time before turning in place.
	if (FMath::Abs(AimingValues.AimingAngle.X) <= TurnInPlaceValues.TurnCheckMinAngle ||
		CharacterInformation.AimYawRate >= TurnInPlaceValues.AimYawRateLimit)
	{
		TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		return;
	}

	TurnInPlaceValues.ElapsedDelayTime += DeltaSeconds;
	const float ClampedAimAngle = FMath::GetMappedRangeValueClamped<float, float>({TurnInPlaceValues.TurnCheckMinAngle, 180.0f},
	                                                                {
		                                                                TurnInPlaceValues.MinAngleDelay,
		                                                                TurnInPlaceValues.MaxAngleDelay
	                                                                },
	                                                                AimingValues.AimingAngle.X);

	// Step 2: Check if the Elapsed Delay time exceeds the set delay (mapped to the turn angle range). If so, trigger a Turn In Place.
	if (TurnInPlaceValues.ElapsedDelayTime > ClampedAimAngle)
	{
		FRotator TurnInPlaceYawRot = CharacterInformation.ViewRotation;
		TurnInPlaceYawRot.Roll = 0.0f;
		TurnInPlaceYawRot.Pitch = 0.0f;
		TurnInPlace(TurnInPlaceYawRot, 1.0f, 0.0f, false);
	}
}

void UOrganicAnimInstance::DynamicTransitionCheck()
{
	// Check each foot to see if the location difference between the IK_Foot bone and its desired / target location
	// (determined via a virtual bone) exceeds a threshold. If it does, play an additive transition animation on that foot.
	// The currently set transition plays the second half of a 2 foot transition animation, so that only a single foot moves.
	// Because only the IK_Foot bone can be locked, the separate virtual bone allows the system to know its desired location when locked.
	FTransform SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootL_BoneName, RTS_Component);
	FTransform SocketTransformB = GetOwningComponent()->GetSocketTransform(
		NAME_VB___foot_target_l, RTS_Component);
	float Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FOrganicDynamicMontage Params;
		Params.Animation = TransitionAnim_R;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}

	SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootR_BoneName, RTS_Component);
	SocketTransformB = GetOwningComponent()->GetSocketTransform(NAME_VB___foot_target_r, RTS_Component);
	Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FOrganicDynamicMontage Params;
		Params.Animation = TransitionAnim_L;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}
}

void UOrganicAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	const FOrganicVelocityBlend& TargetBlend = CalculateVelocityBlend();
	VelocityBlend.Forward = FMath::FInterpTo(VelocityBlend.Forward, TargetBlend.Forward, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.Backward = FMath::FInterpTo(VelocityBlend.Backward, TargetBlend.Backward, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.Left = FMath::FInterpTo(VelocityBlend.Left, TargetBlend.Left, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.Right = FMath::FInterpTo(VelocityBlend.Right, TargetBlend.Right, DeltaSeconds, Config.VelocityBlendInterpSpeed);

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

	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.ViewRotation;
	Delta.Normalize();
	const FVector& FBOffset = YawOffset_FB->GetVectorValue(Delta.Yaw);
	Grounded.FYaw = FBOffset.X;
	Grounded.BYaw = FBOffset.Y;
	const FVector& LROffset = YawOffset_LR->GetVectorValue(Delta.Yaw);
	Grounded.LYaw = LROffset.X;
	Grounded.RYaw = LROffset.Y;
}

void UOrganicAnimInstance::UpdateInAirValues(float DeltaSeconds)
{
	InAir.FallSpeed = CharacterInformation.Velocity.Z;
	InAir.LandPrediction = CalculateLandPrediction();

	const FOrganicLeanAmount& InAirLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, InAirLeanAmount.LR, DeltaSeconds, Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, InAirLeanAmount.FB, DeltaSeconds, Config.GroundedLeanInterpSpeed);
}

void UOrganicAnimInstance::UpdateRagdollValues()
{
	const float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(NAME__ALSCharacterAnimInstance__root).Size();
	FlailRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

float UOrganicAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin,
                                                     float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

FOrganicVelocityBlend UOrganicAnimInstance::CalculateVelocityBlend() const
{
	const FVector LocRelativeVelocityDir =
		CharacterInformation.Rotation.UnrotateVector(CharacterInformation.Velocity.GetSafeNormal(0.1f));
	const float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) +
		FMath::Abs(LocRelativeVelocityDir.Z);
	const FVector RelativeDir = LocRelativeVelocityDir / Sum;
	FOrganicVelocityBlend Result;
	Result.Forward = FMath::Clamp(RelativeDir.X, 0.0f, 1.0f);
	Result.Backward = FMath::Abs(FMath::Clamp(RelativeDir.X, -1.0f, 0.0f));
	Result.Left = FMath::Abs(FMath::Clamp(RelativeDir.Y, -1.0f, 0.0f));
	Result.Right = FMath::Clamp(RelativeDir.Y, 0.0f, 1.0f);
	return Result;
}

FVector UOrganicAnimInstance::CalculateRelativeAccelerationAmount() const
{
	if (FVector::DotProduct(CharacterInformation.Acceleration, CharacterInformation.Velocity) > 0.0f)
	{
		const float MaxAcc = OrganicPawn->GetOrganicMovement()->GetInputAcceleration();
		return CharacterInformation.Rotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
	}

	const float MaxBrakingDec = OrganicPawn->GetOrganicMovement()->GetBrakingDeceleration();
	return
		CharacterInformation.Rotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

float UOrganicAnimInstance::CalculateStrideBlend() const
{
	const float CurveTime = CharacterInformation.Speed / GetOwningComponent()->GetComponentScale().Z;
	const float ClampedGait = GetAnimCurveClamped(NAME_W_Gait, -1.0, 0.0f, 1.0f);
	const float LerpedStrideBlend =
		FMath::Lerp(StrideBlend_N_Walk->GetFloatValue(CurveTime), StrideBlend_N_Run->GetFloatValue(CurveTime),
		            ClampedGait);
	return FMath::Lerp(LerpedStrideBlend, StrideBlend_C_Walk->GetFloatValue(CharacterInformation.Speed),
	                   GetCurveValue(NAME_BasePose_CLF));
}

float UOrganicAnimInstance::CalculateWalkRunBlend() const
{
	return Gait.Walking() ? 0.0f : 1.0;
}

float UOrganicAnimInstance::CalculateStandingPlayRate() const
{
	const float LerpedSpeed = FMath::Lerp(CharacterInformation.Speed / Config.AnimatedWalkSpeed,
	                                      CharacterInformation.Speed / Config.AnimatedRunSpeed,
	                                      GetAnimCurveClamped(NAME_W_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpedSpeed, CharacterInformation.Speed / Config.AnimatedSprintSpeed,
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
		CharacterInformation.Speed / Config.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
		GetComponentScale().Z,
		0.0f, 2.0f);
}

float UOrganicAnimInstance::CalculateLandPrediction() const
{
	if (InAir.FallSpeed >= -200.0f)
	{
		return 0.0f;
	}

	const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(OrganicPawn->GetCapsuleComponent());
	check(CapsuleComp);
	
	const FVector& CapsuleWorldLoc = CapsuleComp->GetComponentLocation();
	const float VelocityZ = CharacterInformation.Velocity.Z;
	FVector VelocityClamped = CharacterInformation.Velocity;
	VelocityClamped.Z = FMath::Clamp(VelocityZ, -4000.0f, -200.0f);
	VelocityClamped.Normalize();

	const FVector TraceLength = VelocityClamped * FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, -4000.0f}, {50.0f, 2000.0f}, VelocityZ);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OrganicPawn);

	FHitResult HitResult;
	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(CapsuleComp->GetUnscaledCapsuleRadius(),
	                                                                           CapsuleComp->GetUnscaledCapsuleHalfHeight());
	const bool bHit = World->SweepSingleByChannel(HitResult, CapsuleWorldLoc, CapsuleWorldLoc + TraceLength, FQuat::Identity,
	                                              ECC_Visibility, CapsuleCollisionShape, Params);

	/*
	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
		                                                CapsuleWorldLoc,
		                                                CapsuleWorldLoc + TraceLength,
		                                                CapsuleCollisionShape,
		                                                EDrawDebugTrace::Type::ForOneFrame,
		                                                bHit,
		                                                HitResult,
		                                                FLinearColor::Red,
		                                                FLinearColor::Green,
		                                                5.0f);
	}
	*/

	if (OrganicPawn->GetOrganicMovement()->HitWalkableFloor(HitResult))
	{
		return FMath::Lerp(LandPredictionCurve->GetFloatValue(HitResult.Time), 0.0f,
		                   GetCurveValue(NAME_Mask_LandPrediction));
	}

	return 0.0f;
}

FOrganicLeanAmount UOrganicAnimInstance::CalculateAirLeanAmount() const
{
	FOrganicLeanAmount CalcLeanAmount;
	const FVector& UnrotatedVel = CharacterInformation.Rotation.UnrotateVector(
		CharacterInformation.Velocity) / 350.0f;
	FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
	InversedVect *= LeanInAirCurve->GetFloatValue(InAir.FallSpeed);
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

	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.ViewRotation;
	Delta.Normalize();
	return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

void UOrganicAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime,
                                            bool OverrideCurrent)
{
	FRotator Delta = TargetRotation - CharacterInformation.Rotation;
	Delta.Normalize();
	const float TurnAngle = Delta.Yaw;

	FOrganicTurnInPlaceAsset TargetTurnAsset;
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

void UOrganicAnimInstance::OnJumped()
{
	InAir.bJumped = true;
	InAir.JumpPlayRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 600.0f}, {1.2f, 1.5f}, CharacterInformation.Speed);

	GetWorld()->GetTimerManager().SetTimer(OnJumpedTimer, this,
	                                  &UOrganicAnimInstance::OnJumpedDelay, 0.1f, false);
}

void UOrganicAnimInstance::OnPivot()
{
	Grounded.bPivot = CharacterInformation.Speed < Config.TriggerPivotSpeedLimit;
	GetWorld()->GetTimerManager().SetTimer(OnPivotTimer, this,
	                                  &UOrganicAnimInstance::OnPivotDelay, 0.1f, false);
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
