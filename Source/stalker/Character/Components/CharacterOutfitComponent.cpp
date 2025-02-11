// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterOutfitComponent.h"
#include "CharacterStateComponent.h"
#include "GameData.h"
#include "ItemBehaviorSet.h"
#include "StalkerCharacter.h"
#include "Components/HitScanComponent.h"

UCharacterOutfitComponent::UCharacterOutfitComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UCharacterOutfitComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner())
	{
		if (const auto GameData = UGameData::Get(GetOwner()))
		{
			ItemBehavior = GameData->GameItemSystemData.ItemBehavior;
		}
	}
}

void UCharacterOutfitComponent::SetupOutfitComponent()
{
	AStalkerCharacter* Character = GetOwner<AStalkerCharacter>();
	if (!IsValid(Character))
	{
		UE_LOG(LogCharacter, Error,
			   TEXT(
				   "Unable to setup Outfit Component ('%s') for character '%s': character ref is not valid."
			   ), *GetName(), *GetOwner()->GetName());
		return;
	}

	CharacterRef = Character;
	check(CharacterRef);

	AbilityComponentRef = CharacterRef->GetAbilitySystemComponent();
	InventoryComponentRef = CharacterRef->GetInventoryComponent();
	HitScanComponentRef = CharacterRef->GetHitScanComponent();
	StateComponentRef = CharacterRef->GetStateComponent();

	if (HasAuthority())
	{
		if (HitScanComponentRef)
		{
			HitScanComponentRef->OnOwnerDamagedDelegate.AddUObject(this, &ThisClass::OnCharacterDamaged);
		}
		
		if (StateComponentRef)
		{
			StateComponentRef->OnOwnerDeadDelegate.AddUObject(this, &ThisClass::OnCharacterDead);
		}
	}
	
	Super::SetupOutfitComponent();
}

void UCharacterOutfitComponent::OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
                                                   const FHitResult& HitResult, float DamageValue)
{
}

void UCharacterOutfitComponent::OnCharacterDead()
{
}

const UItemBehaviorSet* UCharacterOutfitComponent::GetItemBehaviorSet() const
{
	return ItemBehavior.LoadSynchronous();
}
