// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "AbilitySet.h"
#include "CharacterArmorComponent.h"
#include "CharacterStateComponent.h"
#include "CharacterWeaponComponent.h"
#include "EquipmentComponent.h"
#include "InventoryComponent.h"
#include "StalkerCharacterMovementComponent.h"
#include "Components/OrganicAbilityComponent.h"

DEFINE_LOG_CATEGORY(LogCharacter);

FName AStalkerCharacter::CharacterMovementName	{"Char Movement Component"};
FName AStalkerCharacter::InventoryComponentName	{"Char Inventory Component"};
FName AStalkerCharacter::EquipmentComponentName	{"Char Equipment Component"};
FName AStalkerCharacter::WeaponComponentName	{"Char Weapon Component"};
FName AStalkerCharacter::StateComponentName		{"Char State Component"};
FName AStalkerCharacter::ArmorComponentName		{"Char Armor Component"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CharacterMovementComponent = CreateDefaultSubobject<UStalkerCharacterMovementComponent>(CharacterMovementName);
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->UpdatedComponent = GetRootComponent();
	}
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(EquipmentComponentName);
	WeaponComponent = CreateDefaultSubobject<UCharacterWeaponComponent>(WeaponComponentName);
	StateComponent = CreateDefaultSubobject<UCharacterStateComponent>(StateComponentName);
	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);
	
	ID_Action1 = "Jump";
	ID_Action2 = "Crouch";
	ID_Action3 = "Sprint";
}

void AStalkerCharacter::PostInitializeComponents()
{
	if (GetCharacterMovement())
	{
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
	}

	if (GetWeaponComponent())
	{
		GetWeaponComponent()->SetupOutfitComponent(this);
	}

	if (GetStateComponent())
	{
		GetStateComponent()->SetupStateComponent(this);
	}

	if (GetArmorComponent())
	{
		GetArmorComponent()->SetupOutfitComponent(this);
	}

	if (HasAuthority())
	{
		SetCharacterData();
	}
	
	Super::PostInitializeComponents();
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitCharacterComponents();
	
	if (IsLocallyControlled())
	{
		SetupCharacterLocally();
	}
}

void AStalkerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	InitCharacterComponents();
	
	if (IsLocallyControlled())
	{
		SetupCharacterLocally();
	}
}

void AStalkerCharacter::InitCharacterComponents()
{
	if (!GetController())
	{
		return;
	}
	
	if (auto AbilitySystemComp = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		AbilitySystemComp->SetupAbilitySystem(GetController(), this);
	}
	
	if (GetWeaponComponent())
	{
		GetWeaponComponent()->InitCharacterInfo(GetController());
	}
	
	if (GetStateComponent())
	{
		GetStateComponent()->InitCharacterInfo(GetController());
	}
	
	if (GetArmorComponent())
	{
		GetArmorComponent()->InitCharacterInfo(GetController());
	}
}

void AStalkerCharacter::SetupCharacterLocally()
{
}

void AStalkerCharacter::InteractWithContainer(UInventoryComponent* TargetInventory)
{
	OnLootInventory.Broadcast(TargetInventory);
}

void AStalkerCharacter::InteractWithItem(UItemObject* ItemObject)
{
	OnPickUpItem.Broadcast(ItemObject);
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
		TArray<FGameplayAbilitySpecHandle> OutHandles;
		AbilitySet->GiveToAbilitySystem(AbilityComp, OutHandles);
	}
}
