// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArmorComponent.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UCharacterArmorComponent::UCharacterArmorComponent()
{
}

void UCharacterArmorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, EquippedHelmetData,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, EquippedBodyData,	COND_OwnerOnly);
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
			EquippedHelmetData = FEquippedArmorData(IncomingItem, *ArmorBehConfig);
			break;
		}
	case EArmorType::Body:
		{
			EquippedBodyData = FEquippedArmorData(IncomingItem, *ArmorBehConfig);
			break;
		}
	default:
		break;
	}
}

void UCharacterArmorComponent::OnUnequipSlot(const FString& SlotName, UItemObject* OutgoingItem)
{
	if (!OutgoingItem || SlotName.IsEmpty())
	{
		return;
	}

	const FArmorBehavior* ArmorBehConfig = GetArmorBehavior(OutgoingItem->GetScriptName());
	if (!ArmorBehConfig)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *OutgoingItem->GetScriptName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}

	switch (ArmorBehConfig->ArmorType)
	{
	case EArmorType::Helmet:
		{
			EquippedHelmetData.Clear();
			break;
		}
	case EArmorType::Body:
		{
			EquippedBodyData.Clear();
			break;
		}
	default:
		break;
	}
}

USkeletalMeshComponent* UCharacterArmorComponent::GetCharacterMesh() const
{
	if (GetCharacter())
	{
		return GetCharacter()->GetMesh();
	}
	return nullptr;
}
