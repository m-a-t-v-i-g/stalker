// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Movement/OrganicMovementComponent.h"
#include "Characters/BaseOrganic.h"

UOrganicMovementComponent::UOrganicMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOrganicMovementComponent::PostLoad()
{
	Super::PostLoad();

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
	
}
