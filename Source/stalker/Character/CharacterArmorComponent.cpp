// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArmorComponent.h"
#include "ItemObject.h"
#include "Kismet/KismetSystemLibrary.h"

UCharacterArmorComponent::UCharacterArmorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const FArmorBehavior* UCharacterArmorComponent::GetArmorBehavior(const FName& ItemScriptName) const
{
	if (ItemBehaviorConfig)
	{
		if (const FArmorBehavior* Behavior = ItemBehaviorConfig->Armors.Find(ItemScriptName))
		{
			return Behavior;
		}
	}
	return nullptr;
}

void UCharacterArmorComponent::OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem)
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

void UCharacterArmorComponent::OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem)
{
}
