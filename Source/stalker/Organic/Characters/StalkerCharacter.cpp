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
	
	if (auto CharacterInventory = GetInventoryComponent<UCharacterInventoryComponent>())
	{
		CharacterInventory->PreInitializeContainer();

		if (auto CharacterWeapon = GetWeaponComponent<UCharacterWeaponComponent>())
		{
			CharacterWeapon->PreInitializeWeapon();
			CharacterWeapon->PostInitializeWeapon();
		}

		if (GetArmorComponent())
		{
			GetArmorComponent()->PreInitializeArmor();
			GetArmorComponent()->PostInitializeArmor();
		}
		
		CharacterInventory->PostInitializeContainer();
	}
}
