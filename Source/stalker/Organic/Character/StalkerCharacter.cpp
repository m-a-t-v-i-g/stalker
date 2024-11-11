// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "AbilitySet.h"
#include "CharacterArmorComponent.h"
#include "CharacterInventoryComponent.h"
#include "CharacterStateComponent.h"
#include "CharacterWeaponComponent.h"
#include "StalkerCharacterMovementComponent.h"
#include "Components/OrganicAbilityComponent.h"

DEFINE_LOG_CATEGORY(LogCharacter);

FName AStalkerCharacter::InventoryComponentName	{"Inventory Component"};
FName AStalkerCharacter::WeaponComponentName	{"Weapon Component"};
FName AStalkerCharacter::StateComponentName		{"Char State Component"};
FName AStalkerCharacter::ArmorComponentName		{"Char Armor Component"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(InventoryComponentName);
	
	WeaponComponent = CreateDefaultSubobject<UCharacterWeaponComponent>(WeaponComponentName);
	
	StateComponent = CreateDefaultSubobject<UCharacterStateComponent>(StateComponentName);
	
	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);
	
	ID_Action1 = "Jump";
	ID_Action2 = "Crouch";
	ID_Action3 = "Sprint";
}

void AStalkerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		SetCharacterData();
	}
	
	if (WeaponComponent)
	{
		WeaponComponent->SetupWeaponComponent();
	}
	
	if (StateComponent)
	{
		StateComponent->SetupStateComponent();
	}
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (auto AbilitySystemComp = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		AbilitySystemComp->SetupAbilitySystem(NewController, this);
	}
	
	if (IsLocallyControlled())
	{
		SetupCharacterLocally();
	}
}

void AStalkerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	if (IsLocallyControlled())
	{
		SetupCharacterLocally();
	}
}

void AStalkerCharacter::SetupCharacterLocally()
{
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

bool AStalkerCharacter::CheckReloadAbility()
{
	return true;
}
