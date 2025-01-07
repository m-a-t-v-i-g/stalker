// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerCharacter.h"
#include "CharacterArmorComponent.h"
#include "CharacterStateComponent.h"
#include "CharacterWeaponComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "InputMappingContext.h"
#include "InventoryComponent.h"
#include "ItemsContainer.h"
#include "PawnInteractionComponent.h"
#include "StalkerCharacterMovementComponent.h"
#include "StalkerGameplayTags.h"
#include "StalkerInputComponent.h"
#include "Animation/AnimationCore.h"
#include "Components/HitScanComponent.h"

DEFINE_LOG_CATEGORY(LogCharacter);

FName AStalkerCharacter::CharacterMovementName		{"Char Movement Component"};
FName AStalkerCharacter::InventoryComponentName		{"Char Inventory Component"};
FName AStalkerCharacter::EquipmentComponentName		{"Char Equipment Component"};
FName AStalkerCharacter::WeaponComponentName		{"Char Weapon Component"};
FName AStalkerCharacter::StateComponentName			{"Char State Component"};
FName AStalkerCharacter::ArmorComponentName			{"Char Armor Component"};
FName AStalkerCharacter::InteractionComponentName	{"Char Interaction Component"};

AStalkerCharacter::AStalkerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CharacterMovementComponent = CreateDefaultSubobject<UStalkerCharacterMovementComponent>(CharacterMovementName);
	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->UpdatedComponent = GetRootComponent();
	}
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	if (InventoryComponent)
	{
		auto ItemsContainer = CreateDefaultSubobject<UItemsContainer>("Backpack");
		ItemsContainer->SetupItemsContainer(FGameplayTagContainer::EmptyContainer);
		InventoryComponent->SetItemsContainer(ItemsContainer);
	}
	
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(EquipmentComponentName);
	if (EquipmentComponent)
	{
		auto PrimarySlot = CreateDefaultSubobject<UEquipmentSlot>("Primary Slot");
		PrimarySlot->SetupEquipmentSlot("Primary", FGameplayTagContainer::EmptyContainer);
		EquipmentComponent->AddEquipmentSlot(PrimarySlot);
		
		auto SecondarySlot = CreateDefaultSubobject<UEquipmentSlot>("Secondary Slot");
		SecondarySlot->SetupEquipmentSlot("Secondary", FGameplayTagContainer::EmptyContainer);
		EquipmentComponent->AddEquipmentSlot(SecondarySlot);
		
		auto BodyArmorSlot = CreateDefaultSubobject<UEquipmentSlot>("Body Armor Slot");
		BodyArmorSlot->SetupEquipmentSlot("Body", FGameplayTagContainer::EmptyContainer);
		EquipmentComponent->AddEquipmentSlot(BodyArmorSlot);
	}
	
	WeaponComponent = CreateDefaultSubobject<UCharacterWeaponComponent>(WeaponComponentName);
	StateComponent = CreateDefaultSubobject<UCharacterStateComponent>(StateComponentName);
	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);
	InteractionComponent = CreateDefaultSubobject<UPawnInteractionComponent>(InteractionComponentName);
	
	if (GetHitScanComponent())
	{
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::NAME_Pelvis, FStalkerGameplayTags::ArmorPartTag_Body);
	}
	
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

	Super::PostInitializeComponents();
}

void AStalkerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (InteractionComponent)
	{
		InteractionComponent->SetupInteractionComponent();
	}
}

void AStalkerCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	if (InteractionComponent)
	{
		InteractionComponent->SetupInteractionComponent();
	}
}

void AStalkerCharacter::BindDirectionalInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Move, ETriggerEvent::Triggered,
										   this, &ThisClass::Moving);
	}

	Super::BindDirectionalInput(PlayerInputComponent);
}

void AStalkerCharacter::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Slot,
										   ETriggerEvent::Triggered, this, &ThisClass::ToggleEquippedSlot);
	}

	Super::BindKeyInput(PlayerInputComponent);
}

void AStalkerCharacter::ToggleEquippedSlot(const FInputActionInstance& InputAction)
{
	auto PlayerController = GetController<APlayerController>();
	if (!PlayerController || !GetWeaponComponent())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		PlayerController->GetLocalPlayer()))
	{
		TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(InputAction.GetSourceAction());
		
		const FKey* PressedKey = Keys.FindByPredicate([&](const FKey& Key)
		{
			return PlayerController->IsInputKeyDown(Key);
		});

		if (!PressedKey)
		{
			return;
		}

		const TArray<FEnhancedActionKeyMapping> Mappings;// = InputMappingContext->GetMappings();

		for (uint8 i = 0, a = 0; i < Mappings.Num(); i++)
		{
			if (Mappings[i].Action != InputAction.GetSourceAction())
			{
				continue;
			}

			if (Mappings[i].Key == *PressedKey)
			{
				GetWeaponComponent()->ToggleSlot(a);
				break;
			}

			a++;
		}
	}
}
