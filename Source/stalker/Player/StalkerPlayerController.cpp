// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/StalkerPlayerController.h"
#include "PlayerHUD.h"
#include "Components/Items/ItemsContainerComponent.h"

AStalkerPlayerController::AStalkerPlayerController()
{
}

void AStalkerPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AStalkerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}
