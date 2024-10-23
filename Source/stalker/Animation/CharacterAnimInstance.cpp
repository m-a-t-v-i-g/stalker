// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/CharacterAnimInstance.h"
#include "AnimationCore.h"
#include "CharacterAnimConfig.h"
#include "Components/Movement/OrganicMovementComponent.h"
#include "Organic/Characters/BaseCharacter.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABaseCharacter>(TryGetPawnOwner());

	if (Character.IsValid())
	{
		OrganicMovement = Character->GetOrganicMovement();
		
		if (OrganicMovement.IsValid())
		{
			//CollisionShape = MakeCollisionShape();
			//OrganicMovement->OnJumpedDelegate.AddUniqueDynamic(this, &UCharacterLocomotionAnimComponent::OnJumped);
		}
	}
}

void UCharacterAnimInstance::UpdateMovementInfo(float DeltaSeconds)
{
	if (!Character.IsValid()) return;
	
	MovementAction = Character->GetMovementAction();
	OverlayState = Character->GetOverlayState();
	
	// LayerBlendingValues.OverlayOverrideState = Character->GetOverlayOverrideState();
	
	Super::UpdateMovementInfo(DeltaSeconds);

	UpdateLayerValues(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);
}

void UCharacterAnimInstance::UpdateViewInfo(float DeltaSeconds)
{
	Super::UpdateViewInfo(DeltaSeconds);
}

void UCharacterAnimInstance::UpdateLayerValues(float DeltaSeconds)
{
	LayerBlendingValues.EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(FCharacterCurveName::NAME_Mask_AimOffset));

	LayerBlendingValues.BasePose_N = GetCurveValue(FCharacterCurveName::NAME_BasePose_N);
	LayerBlendingValues.BasePose_CLF = GetCurveValue(FCharacterCurveName::NAME_BasePose_C);

	LayerBlendingValues.Spine_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Spine_Add);
	LayerBlendingValues.Head_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Head_Add);
	LayerBlendingValues.Arm_L_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_L_Add);
	LayerBlendingValues.Arm_R_Add = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_R_Add);
	LayerBlendingValues.Hand_L = GetCurveValue(FCharacterLayerName::NAME_Layering_Hand_L);
	LayerBlendingValues.Hand_R = GetCurveValue(FCharacterLayerName::NAME_Layering_Hand_R);

	LayerBlendingValues.EnableHandIK_L = FMath::Lerp(0.0f, GetCurveValue(FCharacterCurveName::NAME_Enable_HandIK_L),
													 GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_L));
	LayerBlendingValues.EnableHandIK_R = FMath::Lerp(0.0f, GetCurveValue(FCharacterCurveName::NAME_Enable_HandIK_R),
													 GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_R));

	LayerBlendingValues.Arm_L_LS = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_L_LS);
	LayerBlendingValues.Arm_L_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_L_LS));
	LayerBlendingValues.Arm_R_LS = GetCurveValue(FCharacterLayerName::NAME_Layering_Arm_R_LS);
	LayerBlendingValues.Arm_R_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_R_LS));
}

void UCharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	SetFootLocking(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_L, FCharacterCurveName::NAME_FootLock_L,
				   FCharacterBoneName::NAME_IkFoot_L, FootIKValues.FootLock_L_Alpha, FootIKValues.UseFootLockCurve_L,
				   FootIKValues.FootLock_L_Location, FootIKValues.FootLock_L_Rotation);
	SetFootLocking(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_R, FCharacterCurveName::NAME_FootLock_R,
				   FCharacterBoneName::NAME_IkFoot_R, FootIKValues.FootLock_R_Alpha, FootIKValues.UseFootLockCurve_R,
				   FootIKValues.FootLock_R_Location, FootIKValues.FootLock_R_Rotation);

	if (MovementState.Airborne())
	{
		SetPelvisIKOffset(DeltaSeconds, FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets(DeltaSeconds);
	}
	else if (!MovementState.Ragdoll())
	{
		SetFootOffsets(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_L, FCharacterBoneName::NAME_IkFoot_L,
		               FCharacterBoneName::NAME_Root, FootOffsetLTarget,
		               FootIKValues.FootOffset_L_Location, FootIKValues.FootOffset_L_Rotation);
		SetFootOffsets(DeltaSeconds, FCharacterCurveName::NAME_Enable_FootIK_R, FCharacterBoneName::NAME_IkFoot_R,
		               FCharacterBoneName::NAME_Root, FootOffsetRTarget,
		               FootIKValues.FootOffset_R_Location, FootIKValues.FootOffset_R_Rotation);
		SetPelvisIKOffset(DeltaSeconds, FootOffsetLTarget, FootOffsetRTarget);
	}
}

void UCharacterAnimInstance::UpdateGroundedValues(float DeltaSeconds)
{
	Super::UpdateGroundedValues(DeltaSeconds);

	if (!Grounded.bShouldMove)
	{
		if (CanDynamicTransition())
		{
			DynamicTransitionCheck();
		}
	}
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
	FootIKValues.PelvisAlpha = (GetCurveValue(FCharacterCurveName::NAME_Enable_FootIK_L) + GetCurveValue(
		FCharacterCurveName::NAME_Enable_FootIK_R)) / 2.0f;

	if (FootIKValues.PelvisAlpha > 0.0f)
	{
		FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;
		float InterpSpeed = PelvisTarget.Z > FootIKValues.PelvisOffset.Z ? 10.0f : 15.0f;

		FootIKValues.PelvisOffset = FMath::VInterpTo(FootIKValues.PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
	}
	else
	{
		FootIKValues.PelvisOffset = FVector::ZeroVector;
	}
}

void UCharacterAnimInstance::ResetIKOffsets(float DeltaSeconds)
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
		Params.Animation = TransitionAnim_R;
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
		Params.Animation = TransitionAnim_L;
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
