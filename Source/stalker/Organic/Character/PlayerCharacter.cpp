// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "CharacterInventoryComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StalkerGameplayTags.h"
#include "Components/OrganicAbilityComponent.h"
#include "Components/PawnInteractionComponent.h"
#include "Input/StalkerInputComponent.h"
#include "Player/PlayerInventoryManagerComponent.h"

FName APlayerCharacter::InteractionComponentName		{"Interaction Component"};
FName APlayerCharacter::InventoryManagerComponentName	{"Inventory Manager Component"};

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InteractionComponent = CreateDefaultSubobject<UPawnInteractionComponent>(InteractionComponentName);

	InventoryManager = CreateDefaultSubobject<UPlayerInventoryManagerComponent>(InventoryManagerComponentName);
}

bool APlayerCharacter::ContainerInteract(UInventoryComponent* TargetInventory)
{
	OnContainerInteraction.Broadcast(TargetInventory);
	ClientContainerInteract(TargetInventory);
	return true;
}

void APlayerCharacter::ClientContainerInteract_Implementation(UInventoryComponent* TargetInventory)
{
	OnContainerInteraction.Broadcast(TargetInventory);
}

bool APlayerCharacter::ItemInteract(UItemObject* ItemObject)
{
	if (GetInventoryComponent())
	{
		GetInventoryComponent()->ServerFindAvailablePlace(ItemObject);
		OnItemInteraction.Broadcast(ItemObject);
		return true;
	}
	return false;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetController<APlayerController>()->GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = 1;
		
		Subsystem->AddMappingContext(InputMappingContext, 1, Options);
	}
	
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::InitCharacterComponents()
{
	Super::InitCharacterComponents();

	if (InventoryManager)
	{
		InventoryManager->SetupInventoryManager(this);
	}
}

void APlayerCharacter::SetupCharacterLocally()
{
	Super::SetupCharacterLocally();

	if (InteractionComponent)
	{
		InteractionComponent->SetupInteractionComponent();
	}
}

void APlayerCharacter::BindDirectionalInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Move, ETriggerEvent::Triggered,
										   this, &APlayerCharacter::IA_Move);
	}
}

void APlayerCharacter::BindViewInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_View, ETriggerEvent::Triggered,
										   this, &APlayerCharacter::IA_View);
	}
}

void APlayerCharacter::BindKeyInput(UInputComponent* PlayerInputComponent)
{
	if (UStalkerInputComponent* StalkerInputComp = Cast<UStalkerInputComponent>(PlayerInputComponent))
	{
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Inventory, ETriggerEvent::Triggered,
		                                   this, &APlayerCharacter::IA_Inventory);
		StalkerInputComp->BindNativeAction(InputConfig, FStalkerGameplayTags::InputTag_Slot, ETriggerEvent::Triggered,
										   this, &APlayerCharacter::IA_Slot);
		
		TArray<uint32> BindHandles;
		StalkerInputComp->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,
		                                     &ThisClass::Input_AbilityInputTagReleased, BindHandles);
	}
}

void APlayerCharacter::IA_Move(const FInputActionValue& Value)
{
	Super::MoveForward(Value.Get<FVector2D>().X);
	Super::MoveRight(Value.Get<FVector2D>().Y);
}

void APlayerCharacter::IA_View(const FInputActionValue& Value)
{
	Super::TurnView(Value.Get<FVector2D>().X);
	Super::PitchView(Value.Get<FVector2D>().Y);
}

void APlayerCharacter::IA_Inventory(const FInputActionValue& Value)
{
	OnPlayerToggleInventory.Broadcast();
}

void APlayerCharacter::IA_Slot(const FInputActionInstance& InputAction)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetController<APlayerController>()->GetLocalPlayer()))
	{
		TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(InputAction.GetSourceAction());
		const FKey* PressedKey = Keys.FindByPredicate([&](const FKey& Key)
		{
			return GetController<APlayerController>()->IsInputKeyDown(Key);
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
				OnPlayerToggleSlot.Broadcast(i);
				break;
			}
			a++;
		}
	}
}

void APlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UOrganicAbilityComponent* AbilityComponent = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		AbilityComponent->AbilityInputTagPressed(InputTag);
	}
}

void APlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UOrganicAbilityComponent* AbilityComponent = GetAbilitySystemComponent<UOrganicAbilityComponent>())
	{
		AbilityComponent->AbilityInputTagReleased(InputTag);
	}
}
