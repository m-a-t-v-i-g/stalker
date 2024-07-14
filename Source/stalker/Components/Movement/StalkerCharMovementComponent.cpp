// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharMovementComponent.h"

UStalkerCharMovementComponent::UStalkerCharMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStalkerCharMovementComponent::BindReplicationData_Implementation()
{
	Super::BindReplicationData_Implementation();
}
