// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "AbilitySystem/Components/StalkerAbilityComponent.h"
#include "Armor/CharacterArmorComponent.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Movement/StalkerCharacterMovementComponent.h"
#include "Weapons/CharacterWeaponComponent.h"

FName AStalkerCharacter::ArmorComponentName {"CharArmorComp"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UCharacterInventoryComponent>(InventoryComponentName)
	                 .SetDefaultSubobjectClass<UCharacterWeaponComponent>(WeaponComponentName))
{
	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bIsStalkerInitialized)
	{
		return;
	}

	if (auto AbilitySystemComp = GetAbilitySystemComponent<UStalkerAbilityComponent>())
	{
		AbilitySystemComp->InitAbilitySystem(NewController, this);
	}
	
	if (auto CharacterInventory = GetInventoryComponent<UCharacterInventoryComponent>())
	{
		CharacterInventory->PreInitializeContainer();
		
		if (auto CharacterWeapon = GetWeaponComponent<UCharacterWeaponComponent>())
		{
			CharacterWeapon->PreInitializeWeapon();
			CharacterWeapon->PostInitializeWeapon();

			CharacterWeapon->OnAimingStart.AddUObject(this, &AStalkerCharacter::OnAimingStart);
			CharacterWeapon->OnAimingStop.AddUObject(this, &AStalkerCharacter::OnAimingStop);
			CharacterWeapon->OnWeaponOverlayChanged.AddUObject(this, &AStalkerCharacter::OnOverlayChanged);
		}
		
		if (auto CharacterArmor = GetArmorComponent())
		{
			CharacterArmor->PreInitializeArmor();
			CharacterArmor->PostInitializeArmor();
		}
		
		CharacterInventory->PostInitializeContainer();
	}
	
	bIsStalkerInitialized = true;
}

void AStalkerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	if (IsLocallyControlled())
	{
		SetupCharacterLocally(GetController());
	}
}

void AStalkerCharacter::SetupCharacterLocally(AController* NewController)
{
	if (bIsStalkerInitialized)
	{
		return;
	}
	
	if (auto CharacterWeapon = GetWeaponComponent<UCharacterWeaponComponent>())
	{
		CharacterWeapon->PreInitializeWeapon();
		CharacterWeapon->PostInitializeWeapon();

		CharacterWeapon->OnAimingStart.AddUObject(this, &AStalkerCharacter::OnAimingStart);
		CharacterWeapon->OnAimingStop.AddUObject(this, &AStalkerCharacter::OnAimingStop);
		CharacterWeapon->OnWeaponOverlayChanged.AddUObject(this, &AStalkerCharacter::OnOverlayChanged);
	}
	
	bIsStalkerInitialized = true;
}

void AStalkerCharacter::OnAimingStart()
{
	GetOrganicMovement()->SetRotationMode(ECharacterRotationMode::ControlDirection, true);
}

void AStalkerCharacter::OnAimingStop()
{
	GetOrganicMovement()->SetRotationMode(GetOrganicMovement()->GetInputRotationMode(), true);
}

void AStalkerCharacter::OnOverlayChanged(ECharacterOverlayState NewOverlay)
{
	SetOverlayState(NewOverlay);
}

bool AStalkerCharacter::CheckFireAbility()
{
	return true; //!GetWeaponComponent()->IsReloading();
}

bool AStalkerCharacter::CheckAimingAbility()
{
	return true; //!GetWeaponComponent()->IsReloading();
}

bool AStalkerCharacter::CheckReloadAbility()
{
	return true;
}

bool AStalkerCharacter::CheckSprintAbility()
{
	return GetOrganicMovement()->CanSprint();
}

bool AStalkerCharacter::CheckCrouchAbility()
{
	return GetOrganicMovement()->CanCrouch();
}

bool AStalkerCharacter::CheckJumpAbility()
{
	return GetOrganicMovement()->CanJump();
}
