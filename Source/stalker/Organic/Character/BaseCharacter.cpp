// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InventoryComponent.h"
#include "StalkerCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/WeaponComponent.h"

DEFINE_LOG_CATEGORY(LogCharacter);

FName ABaseCharacter::CharacterMovementName {"Char Movement Component"};
FName ABaseCharacter::InventoryComponentName {"Inventory Component"};
FName ABaseCharacter::WeaponComponentName {"Weapon Component"};

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

	if (!GetCharacterMovement())
	{
		return;
	}
	
	if (GetMesh())
	{
		GetMesh()->AddTickPrerequisiteActor(this);
		
		if (GetMesh()->PrimaryComponentTick.bCanEverTick)
		{
			GetMesh()->PrimaryComponentTick.AddPrerequisite(GetCharacterMovement(), GetCharacterMovement()->PrimaryComponentTick);
		}
	}

	if (GetCapsuleComponent())
	{
		GetCharacterMovement()->UpdateNavAgent(*GetCapsuleComponent());
	}

	if (DefaultMovementModel)
	{
		GetCharacterMovement()->SetMovementModel(DefaultMovementModel);
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
