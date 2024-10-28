// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "AbilitySet.h"
#include "CharacterArmorComponent.h"
#include "CharacterInventoryComponent.h"
#include "CharacterWeaponComponent.h"
#include "StalkerCharacterMovementComponent.h"
#include "Components/OrganicAbilityComponent.h"

FName AStalkerCharacter::ArmorComponentName {"Char Armor Component"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UCharacterInventoryComponent>(InventoryComponentName)
	                 .SetDefaultSubobjectClass<UCharacterWeaponComponent>(WeaponComponentName))
{
	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);

	ID_Action1 = "Jump";
	ID_Action2 = "Crouch";
	ID_Action3 = "Sprint";
}

void AStalkerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetCharacterData();
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bIsStalkerInitialized)
	{
		return;
	}

	if (auto AbilitySystemComp = GetAbilitySystemComponent<UOrganicAbilityComponent>())
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

void AStalkerCharacter::SetCharacterData()
{
	auto AbilityComp = GetAbilitySystemComponent<UOrganicAbilityComponent>();
	if (!AbilityComp)
	{
		return;
	}
	
	if (AbilitySet)
	{
		AbilitySet->GiveToAbilitySystem(AbilityComp);
	}
}

void AStalkerCharacter::OnAimingStart()
{
	if (!GetCharacterMovement())
	{
		return;
	}
	
	GetCharacterMovement()->SetRotationMode(ECharacterRotationMode::ControlDirection, true);
}

void AStalkerCharacter::OnAimingStop()
{
	if (!GetCharacterMovement())
	{
		return;
	}
	
	GetCharacterMovement()->SetRotationMode(GetCharacterMovement()->GetInputRotationMode(), true);
}

void AStalkerCharacter::OnOverlayChanged(ECharacterOverlayState NewOverlay)
{
	SetOverlayState(NewOverlay);
}

bool AStalkerCharacter::CheckReloadAbility()
{
	return true;
}
