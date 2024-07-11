// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Movement/OrganicMovementComponent.h"
#include "Organic/BaseOrganic.h"

UOrganicMovementComponent::UOrganicMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOrganicMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);
	OrganicOwner = GetOwner<ABaseOrganic>();
}

void UOrganicMovementComponent::BindReplicationData_Implementation()
{
	Super::BindReplicationData_Implementation();

	BindRotator(ViewRotation, false, true, true);
	BindRotator(PrevPawnRotation, false, true, true);
}

void UOrganicMovementComponent::MovementUpdate_Implementation(float DeltaSeconds)
{
	Super::MovementUpdate_Implementation(DeltaSeconds);

	ViewRotation = FMath::RInterpTo(ViewRotation, GetInControlRotation(), DeltaSeconds, ViewInterpSpeed);
	
	PrevPawnRotation = GetInRotation();
	PrevComponentRotation = GetRootCollisionRotation();
}

void UOrganicMovementComponent::MovementUpdateSimulated_Implementation(float DeltaSeconds)
{
	Super::MovementUpdateSimulated_Implementation(DeltaSeconds);

	PrevComponentRotation = GetRootCollisionRotation();
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
	//bRequestMovementSettingsChange = true;
}

void UOrganicMovementComponent::SetAllowedGait(EOrganicGait DesiredGait)
{
	if (AllowedGait == DesiredGait) return;

	AllowedGait = DesiredGait;
	//bRequestMovementSettingsChange = true;
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
