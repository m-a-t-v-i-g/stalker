// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArmorComponent.h"
#include "CharacterInventoryComponent.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
#include "Components/EquipmentSlot.h"
#include "Data/ArmorBehaviorConfig.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacterArmorComponent::UCharacterArmorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterArmorComponent::SetupArmorComponent(AStalkerCharacter* InCharacter)
{
	CharacterRef = InCharacter;

	if (!CharacterRef)
	{
		UE_LOG(LogCharacter, Error, TEXT("Unable to setup the Armor Component: character ref is null."));
		return;
	}
	
	InventoryComponentRef = CharacterRef->GetInventoryComponent();
	StateComponentRef = CharacterRef->GetStateComponent();
}

void UCharacterArmorComponent::InitCharacterInfo(AController* InController)
{
	ControllerRef = InController;
	
	if (!ControllerRef)
	{
		UE_LOG(LogCharacter, Error,
			   TEXT(
				   "Unable to setup the Armor Component for character: controller ref is null."
			   ));
		return;
	}

	if (IsAuthority())
	{
		if (InventoryComponentRef)
		{
			for (uint8 i = 0; i < ArmorSlots.Num(); i++)
			{
				if (ArmorSlots[i].SlotName.IsEmpty())
				{
					continue;
				}

				if (UEquipmentSlot* SlotPtr = InventoryComponentRef->FindEquipmentSlot(ArmorSlots[i].SlotName))
				{
					SlotPtr->OnSlotChanged.AddUObject(this, &UCharacterArmorComponent::OnSlotEquipped,
													  ArmorSlots[i].SlotName);
					OnSlotEquipped(FUpdatedSlotData(SlotPtr->GetBoundObject(), SlotPtr->IsEquipped()), ArmorSlots[i].SlotName);
				}
			}
		}
	}
}

void UCharacterArmorComponent::OnSlotEquipped(const FUpdatedSlotData& SlotData, FString SlotName)
{
	if (!IsValid(SlotData.SlotItem) || SlotName.IsEmpty())
	{
		return;
	}

	if (SlotData.bIsEquipped)
	{
		EquipSlot(SlotName, SlotData.SlotItem);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot equipped!"), *SlotName), true, false);
	}
	else
	{
		UnequipSlot(SlotName);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot unequipped!"), *SlotName), true, false);
	}
}

bool UCharacterArmorComponent::IsAuthority() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

bool UCharacterArmorComponent::IsAutonomousProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_AutonomousProxy;
}

bool UCharacterArmorComponent::IsSimulatedProxy() const
{
	if (!GetOwner())
	{
		return false;
	}
	return GetOwner()->GetLocalRole() == ROLE_SimulatedProxy;
}

FOutfitSlot* UCharacterArmorComponent::FindArmorSlot(const FString& SlotName)
{
	for (FOutfitSlot& Slot : ArmorSlots)
	{
		if (Slot.SlotName == SlotName)
		{
			return &Slot;
		}
	}
	return nullptr;
}

const FArmorBehavior* UCharacterArmorComponent::GetArmorBehavior(const FName& ItemScriptName) const
{
	if (ArmorBehaviorConfig)
	{
		if (const FArmorBehavior* Behavior = ArmorBehaviorConfig->ItemsMap.Find(ItemScriptName))
		{
			return Behavior;
		}
	}
	return nullptr;
}

void UCharacterArmorComponent::EquipSlot(const FString& SlotName, UItemObject* IncomingItem)
{
	if (!IncomingItem || SlotName.IsEmpty())
	{
		return;
	}

	const FArmorBehavior* ArmorBehConfig = GetArmorBehavior(IncomingItem->GetScriptName());
	if (!ArmorBehConfig)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *IncomingItem->GetScriptName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}

	switch (ArmorBehConfig->ArmorType)
	{
	case EArmorType::Helmet:
		{
			break;
		}
	case EArmorType::Body:
		{
			break;
		}
	default:
		break;
	}
}

void UCharacterArmorComponent::UnequipSlot(const FString& SlotName)
{
	
}
