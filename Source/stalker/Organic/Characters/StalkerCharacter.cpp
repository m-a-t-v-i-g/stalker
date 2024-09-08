// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "Armor/CharacterArmorComponent.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Weapons/CharacterWeaponComponent.h"

FName AStalkerCharacter::ArmorComponentName {"CharArmorComp"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.Get()
	.SetDefaultSubobjectClass<UCharacterInventoryComponent>(InventoryComponentName)
	.SetDefaultSubobjectClass<UCharacterWeaponComponent>(WeaponComponentName))
{
	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);
	
	PrimaryActorTick.bCanEverTick = true;
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bIsStalkerInitialized) return;
	
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
	if (bIsStalkerInitialized) return;
		
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
	SetRotationMode(EOrganicRotationMode::ControlDirection);
}

void AStalkerCharacter::OnAimingStop()
{
	SetRotationMode(EOrganicRotationMode::LookingDirection);
}

void AStalkerCharacter::OnOverlayChanged(ECharacterOverlayState NewOverlay)
{
	SetOverlayState(NewOverlay);
}

bool AStalkerCharacter::CheckReloadAbility()
{
	return !IsSprinting();
}
