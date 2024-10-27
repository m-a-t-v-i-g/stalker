// Fill out your copyright notice in the Description page of Project Settings.

#include "Organic/Characters/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Movement/StalkerCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponComponent.h"

FName ABaseCharacter::CharacterMovementName {"OrganicMoveComp"};
FName ABaseCharacter::InventoryComponentName {"InventoryComp"};
FName ABaseCharacter::WeaponComponentName {"WeaponComp"};

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CharacterMovementComponent = CreateDefaultSubobject<UStalkerCharacterMovementComponent>(CharacterMovementName);
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->UpdatedComponent = GetRootComponent();
	}
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	
	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(WeaponComponentName);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, OverlayState);
}

void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (GetMesh())
	{
		GetMesh()->AddTickPrerequisiteActor(this);
		
		if (GetCharacterMovement() && GetMesh()->PrimaryComponentTick.bCanEverTick)
		{
			GetMesh()->PrimaryComponentTick.AddPrerequisite(GetCharacterMovement(), GetCharacterMovement()->PrimaryComponentTick);
		}
	}

	if (GetCharacterMovement())
	{
		if (GetCapsuleComponent())
		{
			GetCharacterMovement()->UpdateNavAgent(*GetCapsuleComponent());
		}

		if (DefaultMovementModel)
		{
			GetCharacterMovement()->SetMovementModel(DefaultMovementModel);
		}
	}
}

void ABaseCharacter::SetMovementAction(ECharacterMovementAction NewAction, bool bForce)
{
	if (bForce || MovementAction != NewAction)
	{
		const ECharacterMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void ABaseCharacter::OnMovementActionChanged(ECharacterMovementAction PreviousAction)
{
	if (MovementAction != PreviousAction)
	{
		// Logic
	}
}

void ABaseCharacter::SetOverlayState(ECharacterOverlayState NewState, bool bForce)
{
	if (bForce || OverlayState != NewState)
	{
		const ECharacterOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);
	}
}

void ABaseCharacter::OnOverlayStateChanged(ECharacterOverlayState PreviousState)
{
	if (OverlayState != PreviousState)
	{
		// Logic
	}
}
