// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterOutfitComponent.h"

#include "CharacterStateComponent.h"
#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "StalkerCharacter.h"
#include "Components/HitScanComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacterOutfitComponent::UCharacterOutfitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
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
}

void UCharacterOutfitComponent::InitCharacterInfo(AController* InController)
{
	ControllerRef = InController;
	
	if (!IsValid(ControllerRef))
	{
		UE_LOG(LogCharacter, Error,
			   TEXT(
				   "Unable to setup Outfit Component ('%s') for character '%s': controller ref is null."
			   ), *GetName(), *GetOwner()->GetName());
		return;
	}

	if (IsAuthority())
	{
		if (EquipmentComponentRef)
		{
			for (uint8 i = 0; i < OutfitSlots.Num(); i++)
			{
				if (OutfitSlots[i].SlotName.IsEmpty())
				{
					continue;
				}

				if (UEquipmentSlot* SlotPtr = EquipmentComponentRef->FindEquipmentSlot(OutfitSlots[i].SlotName))
				{
					SlotPtr->OnSlotChanged.AddUObject(this, &UCharacterOutfitComponent::OnEquipmentSlotChanged,
													  OutfitSlots[i].SlotName);
					OnEquipmentSlotChanged(FUpdatedSlotData(SlotPtr->GetBoundObject(), SlotPtr->IsEquipped()),
					                       OutfitSlots[i].SlotName);
				}
			}
		}

		if (StateComponentRef)
		{
			StateComponentRef->OnCharacterDead.AddUObject(this, &UCharacterOutfitComponent::OnCharacterDead);
		}
		
		if (HitScanComponentRef)
		{
			HitScanComponentRef->OnOwnerDamagedDelegate.AddUObject(this, &UCharacterOutfitComponent::OnCharacterDamaged);
		}
	}
}

void UCharacterOutfitComponent::OnEquipmentSlotChanged(const FUpdatedSlotData& SlotData, FString SlotName)
{
	if (!IsValid(SlotData.SlotItem) || SlotName.IsEmpty())
	{
		return;
	}

	if (SlotData.bIsEquipped)
	{
		ArmSlot(SlotName, SlotData.SlotItem);
		OnEquipSlot(SlotName, SlotData.SlotItem);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s: %s slot equipped!"), *GetName(), *SlotName),
		                                  true, false);
	}
	else
	{
		OnUnequipSlot(SlotName, SlotData.SlotItem);
		DisarmSlot(SlotName);
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

bool UCharacterOutfitComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UCharacterOutfitComponent::IsAutonomousProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UCharacterOutfitComponent::IsSimulatedProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_SimulatedProxy;
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

void UCharacterOutfitComponent::OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem)
{
}

void UCharacterOutfitComponent::OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem)
{
}
