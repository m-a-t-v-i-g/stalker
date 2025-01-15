// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterOutfitComponent.h"
#include "CharacterStateComponent.h"
#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "GameData.h"
#include "ItemBehaviorSet.h"
#include "StalkerCharacter.h"
#include "Components/HitScanComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacterOutfitComponent::UCharacterOutfitComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	
	SetIsReplicatedByDefault(true);
}

void UCharacterOutfitComponent::SetupOutfitComponent(AStalkerCharacter* InCharacter)
{
	CharacterRef = InCharacter;

	if (!IsValid(CharacterRef))
	{
		UE_LOG(LogCharacter, Error,
		       TEXT(
			       "Unable to setup Outfit Component ('%s') for character '%s': character ref is not valid."
		       ), *GetName(), *GetOwner()->GetName());
		return;
	}

	AbilityComponentRef = CharacterRef->GetAbilitySystemComponent();
	InventoryComponentRef = CharacterRef->GetInventoryComponent();
	EquipmentComponentRef = CharacterRef->GetEquipmentComponent();
	StateComponentRef = CharacterRef->GetStateComponent();
	HitScanComponentRef = CharacterRef->GetHitScanComponent();
	
	if (HasAuthority())
	{
		if (EquipmentComponentRef)
		{
			if (!EquipmentComponentRef->GetEquipmentSlots().IsEmpty())
			{
				for (const FOutfitSlot& OutfitSlot : OutfitSlots)
				{
					if (OutfitSlot.SlotName.IsEmpty())
					{
						continue;
					}

					UEquipmentSlot* SlotPtr = EquipmentComponentRef->FindEquipmentSlot(OutfitSlot.SlotName);
					if (IsValid(SlotPtr))
					{
						OnEquipmentSlotChanged(
							FEquipmentSlotChangeData(SlotPtr->GetSlotName(), SlotPtr->GetBoundObject(),
							                         SlotPtr->IsEquipped()));
						SlotPtr->OnSlotDataChange.AddUObject(this, &UCharacterOutfitComponent::OnEquipmentSlotChanged);
					}
				}
			}
		}

		if (StateComponentRef)
		{
			StateComponentRef->OnOwnerDeadDelegate.AddUObject(this, &UCharacterOutfitComponent::OnCharacterDead);
		}
		
		if (HitScanComponentRef)
		{
			HitScanComponentRef->OnOwnerDamagedDelegate.AddUObject(this, &UCharacterOutfitComponent::OnCharacterDamaged);
		}
	}
}

void UCharacterOutfitComponent::AddOutfitSlot(const FOutfitSlot& OutfitSlot)
{
	OutfitSlots.Add(OutfitSlot);
}

void UCharacterOutfitComponent::OnEquipmentSlotChanged(const FEquipmentSlotChangeData& SlotData)
{
	UItemObject* SlotItem = SlotData.SlotItem;
	const FString& SlotName = SlotData.SlotName;
	bool bIsEquipped = SlotData.bIsEquipped;
	
	if (!IsValid(SlotItem) || SlotName.IsEmpty())
	{
		return;
	}

	if (bIsEquipped)
	{
		ArmSlot(SlotName, SlotItem);
		OnEquipSlot(SlotName, SlotItem);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s: %s slot equipped!"), *GetName(), *SlotName),
		                                  true, false);
	}
	else
	{
		DisarmSlot(SlotName);
		OnUnequipSlot(SlotName, SlotItem);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s: %s slot unequipped!"), *GetName(), *SlotName),
		                                  true, false);
	}
}

void UCharacterOutfitComponent::OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
                                                   const FHitResult& HitResult, float DamageValue)
{
}

void UCharacterOutfitComponent::OnCharacterDead()
{
}

void UCharacterOutfitComponent::ArmSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (!ItemObject)
	{
		return;
	}

	if (FOutfitSlot* Slot = FindOutfitSlot(SlotName))
	{
		Slot->ArmedObject = ItemObject;
	}
}

void UCharacterOutfitComponent::DisarmSlot(const FString& SlotName)
{
	if (FOutfitSlot* Slot = FindOutfitSlot(SlotName))
	{
		if (!Slot->IsArmed())
		{
			return;
		}
		
		Slot->ArmedObject = nullptr;
	}
}

FOutfitSlot* UCharacterOutfitComponent::FindOutfitSlot(const FString& SlotName)
{
	for (FOutfitSlot& Slot : OutfitSlots)
	{
		if (Slot.SlotName == SlotName)
		{
			return &Slot;
		}
	}
	return nullptr;
}

void UCharacterOutfitComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner())
	{
		if (auto Character = GetOwner<AStalkerCharacter>())
		{
			SetupOutfitComponent(Character);
		}

		if (auto GameData = UGameData::Get(GetOwner()))
		{
			ItemBehavior = GameData->GameItemSystemData.ItemBehavior;
		}
	}
}

void UCharacterOutfitComponent::OnEquipSlot(const FString& SlotName, UItemObject* InItem)
{
}

void UCharacterOutfitComponent::OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem)
{
}

const UItemBehaviorSet* UCharacterOutfitComponent::GetItemBehaviorSet() const
{
	return ItemBehavior.LoadSynchronous();
}
