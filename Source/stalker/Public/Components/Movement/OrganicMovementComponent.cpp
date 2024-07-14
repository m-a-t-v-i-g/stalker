// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Movement/OrganicMovementComponent.h"

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

void UOrganicMovementComponent::PreMovementUpdate_Implementation(float DeltaSeconds)
{
	Super::PreMovementUpdate_Implementation(DeltaSeconds);
	
	if (GetIterationNumber() == 1)
	{
		bJustJumped = false;
	}
}

void UOrganicMovementComponent::MovementUpdate_Implementation(float DeltaSeconds)
{
	Super::MovementUpdate_Implementation(DeltaSeconds);

	ViewRotation = FMath::RInterpTo(ViewRotation, GetInControlRotation(), DeltaSeconds, ViewInterpSpeed);

	if (bWantsChangeMovementSettings)
	{
		TargetMaxSpeed = MovementSettings.GetSpeedForGait(AllowedGait);
		bWantsChangeMovementSettings = false;
	}
	
	MaxDesiredSpeed = FMath::FInterpTo(MaxDesiredSpeed, TargetMaxSpeed, DeltaSeconds, SpeedInterpSpeed);
	
	PrevPawnRotation = GetInRotation();
	PrevComponentRotation = GetRootCollisionRotation();

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
	OrganicOwner->OnMovementModeChanged();
}

void UOrganicMovementComponent::OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode)
{
	Super::OnMovementModeChangedSimulated_Implementation(PreviousMovementMode);
	OrganicOwner->OnMovementModeChanged();
}

void UOrganicMovementComponent::SetMovementSettings(FOrganicMovementSettings NewMovementSettings)
{
	MovementSettings = NewMovementSettings;
	bWantsChangeMovementSettings = true;
}

void UOrganicMovementComponent::SetAllowedGait(EOrganicGait DesiredGait)
{
	if (AllowedGait == DesiredGait) return;

	AllowedGait = DesiredGait;
	bWantsChangeMovementSettings = true;
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
