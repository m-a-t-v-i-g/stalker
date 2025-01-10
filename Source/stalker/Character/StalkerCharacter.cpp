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
#include "StalkerPlayerController.h"
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
		FGameplayTagContainer GameplayTags(FItemSystemTags::ItemTag_Item);
		ItemsContainer->SetupItemsContainer(GameplayTags);
		InventoryComponent->SetItemsContainer(ItemsContainer);
	}

	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(EquipmentComponentName);
	if (EquipmentComponent)
	{
		auto PrimarySlot = CreateDefaultSubobject<UEquipmentSlot>("Primary Slot");
		FGameplayTagContainer PrimaryGameplayTags(FItemSystemTags::ItemTag_WeaponPrimary);
		PrimarySlot->SetupEquipmentSlot("Primary", PrimaryGameplayTags);
		EquipmentComponent->AddEquipmentSlot(PrimarySlot);
		
		auto SecondarySlot = CreateDefaultSubobject<UEquipmentSlot>("Secondary Slot");
		FGameplayTagContainer SecondaryGameplayTags(FItemSystemTags::ItemTag_WeaponSecondary);
		SecondarySlot->SetupEquipmentSlot("Secondary", SecondaryGameplayTags);
		EquipmentComponent->AddEquipmentSlot(SecondarySlot);
		
		auto BodyArmorSlot = CreateDefaultSubobject<UEquipmentSlot>("Body Armor Slot");
		FGameplayTagContainer BodyGameplayTags(FItemSystemTags::ItemTag_ArmorBody);
		BodyArmorSlot->SetupEquipmentSlot("Body", BodyGameplayTags);
		EquipmentComponent->AddEquipmentSlot(BodyArmorSlot);
	}

	WeaponComponent = CreateDefaultSubobject<UCharacterWeaponComponent>(WeaponComponentName);
	if (WeaponComponent)
	{
		FOutfitSlot KnifeSlot;
		KnifeSlot.SlotName = "Knife";
		WeaponComponent->AddOutfitSlot(KnifeSlot);

		FOutfitSlot SecondarySlot;
		SecondarySlot.SlotName = "Secondary";
		WeaponComponent->AddOutfitSlot(SecondarySlot);

		FOutfitSlot PrimarySlot;
		PrimarySlot.SlotName = "Primary";
		WeaponComponent->AddOutfitSlot(PrimarySlot);
	}

	ArmorComponent = CreateDefaultSubobject<UCharacterArmorComponent>(ArmorComponentName);
	if (ArmorComponent)
	{
		FOutfitSlot HelmetSlot;
		HelmetSlot.SlotName = "Helmet";
		ArmorComponent->AddOutfitSlot(HelmetSlot);
	
		FOutfitSlot BodySlot;
		BodySlot.SlotName = "Body";
		ArmorComponent->AddOutfitSlot(BodySlot);
	}

	StateComponent = CreateDefaultSubobject<UCharacterStateComponent>(StateComponentName);
	InteractionComponent = CreateDefaultSubobject<UPawnInteractionComponent>(InteractionComponentName);

	if (GetHitScanComponent())
	{
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Pelvis,		FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Spine_01,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Spine_02,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Spine_03,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Spine_04,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Spine_05,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Neck_01,	FStalkerGameplayTags::ArmorPartTag_Helmet);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Neck_02,	FStalkerGameplayTags::ArmorPartTag_Helmet);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Head,		FStalkerGameplayTags::ArmorPartTag_Helmet);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Clavicle_L, FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Upperarm_L, FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Hand_L,		FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Clavicle_R, FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Upperarm_R, FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Hand_R,		FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Thigh_L,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Calf_L,		FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Foot_L,		FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Thigh_R,	FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Calf_R,		FStalkerGameplayTags::ArmorPartTag_Body);
		GetHitScanComponent()->AddBoneToScanMap(FCharacterBoneName::Foot_R,		FStalkerGameplayTags::ArmorPartTag_Body);
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

void AStalkerCharacter::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Slot,
										   ETriggerEvent::Triggered, this, &ThisClass::ToggleSlot);
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Inventory,
		                                   ETriggerEvent::Triggered, this, &ThisClass::ToggleInventory);
	}

	Super::BindKeyInput(PlayerInputComponent);
}

void AStalkerCharacter::ToggleSlot(const FInputActionInstance& InputAction)
{
	auto PlayerController = GetController<AStalkerPlayerController>();
	if (!PlayerController || !GetWeaponComponent())
	{
		return;
	}

	const UInputMappingContext* InputMappingContext = PlayerController->CharacterMappingContext;
	if (!InputMappingContext)
	{
		return;
	}

	if (auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
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

		const TArray<FEnhancedActionKeyMapping>& Mappings = InputMappingContext->GetMappings();
		for (uint8 i = 0, a = 0; i < Mappings.Num(); i++)
		{
			if (Mappings[i].Action != InputAction.GetSourceAction())
			{
				continue;
			}

			if (Mappings[i].Key == *PressedKey)
			{
				ToggleSlotDelegate.Broadcast(a);
				break;
			}

			a++;
		}
	}
}

void AStalkerCharacter::ToggleInventory(const FInputActionInstance& InputAction)
{
	ToggleInventoryDelegate.Broadcast();
}
