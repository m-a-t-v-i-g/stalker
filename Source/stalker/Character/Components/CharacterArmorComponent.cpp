﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArmorComponent.h"
#include "AbilitySystemComponent.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
#include "StalkerGameplayTags.h"
#include "Armor/ArmorObject.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UCharacterArmorComponent::UCharacterArmorComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterArmorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, EquippedHelmetData,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, EquippedBodyData,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, TotalArmorData,		COND_OwnerOnly);
}

void UCharacterArmorComponent::OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
                                                  const FHitResult& HitResult, float DamageValue)
{
	Super::OnCharacterDamaged(DamageTag, PartTag, HitResult, DamageValue);

	UItemObject** DamagedArmorPtr = EquippedArmorParts.Find(PartTag);
	if (!DamagedArmorPtr)
	{
		return;
	}

	if (UItemObject* DamagedArmor = *DamagedArmorPtr)
	{
		DamagedArmor->SpoilEndurance(DamageTag, DamageValue);
	}
}

bool UCharacterArmorComponent::EquipArmor(UItemObject* ItemObject, FEquippedArmorPartData& ArmorData)
{
	if (!GetAbilityComponent())
	{
		return false;
	}

	UArmorObject* ArmorObject = Cast<UArmorObject>(ItemObject);
	if (!ArmorObject)
	{
		return false;
	}

	if (ActiveItemEffects.Contains(ArmorObject))
	{
		return false;
	}

	const FArmorBehavior* ArmorBehConfig = GetArmorBehavior(ArmorObject->GetScriptName());
	if (!ArmorBehConfig)
	{
		return false;
	}

	ApplyItemEffectSpec(ArmorObject);
	ArmorObject->OnEnduranceChangeDelegate.AddUObject(this, &UCharacterArmorComponent::OnEquippedArmorEnduranceChanged,
	                                                   ItemObject);
	ArmorData = FEquippedArmorPartData(ArmorObject, *ArmorBehConfig);
	return true;
}

void UCharacterArmorComponent::UnequipArmor(UItemObject* ItemObject, FEquippedArmorPartData& ArmorData)
{
	if (!GetAbilityComponent())
	{
		return;
	}

	UArmorObject* ArmorObject = Cast<UArmorObject>(ItemObject);
	if (!ArmorObject)
	{
		return;
	}

	RemoveItemEffectSpec(ArmorObject);
	ArmorObject->OnEnduranceChangeDelegate.RemoveAll(this);
	ArmorData.Clear();
}

void UCharacterArmorComponent::OnEquippedArmorEnduranceChanged(float ItemEndurance, UItemObject* ItemObject)
{
	CalculateTotalArmorEndurance();
	ReapplyItemEffectSpec(ItemObject);
}

float UCharacterArmorComponent::CalculateTotalArmorEndurance()
{
	float UpdatedEndurance = 0.0f;
	
	if (!EquippedArmorParts.IsEmpty())
	{
		for (const auto& ArmorPart : EquippedArmorParts)
		{
			if (UItemObject* ArmorObject = ArmorPart.Value)
			{
				UpdatedEndurance += ArmorObject->GetEndurance();
			}
		}
	}

	TotalArmorData.ArmorPartsNum = EquippedArmorParts.Num();
	TotalArmorData.TotalArmorEndurance = UpdatedEndurance;
	OnTotalArmorDataChangedDelegate.Broadcast(TotalArmorData);

	return UpdatedEndurance;
}

const FArmorBehavior* UCharacterArmorComponent::GetArmorBehavior(const FName& ItemScriptName) const
{
	if (GetItemBehavior())
	{
		if (const FArmorBehavior* Behavior = GetItemBehavior()->Armors.Find(ItemScriptName))
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
			if (EquipArmor(IncomingItem, EquippedHelmetData))
			{
				EquippedArmorParts.Add(FStalkerGameplayTags::ArmorPartTag_Helmet, IncomingItem);
			}
			break;
		}
	case EArmorType::Body:
		{
			if (EquipArmor(IncomingItem, EquippedBodyData))
			{
				EquippedArmorParts.Add(FStalkerGameplayTags::ArmorPartTag_Body, IncomingItem);
			}
			break;
		}
	default:
		break;
	}

	CalculateTotalArmorEndurance();
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
			UnequipArmor(OutgoingItem, EquippedHelmetData);
			EquippedArmorParts.Remove(FStalkerGameplayTags::ArmorPartTag_Helmet);
			break;
		}
	case EArmorType::Body:
		{
			UnequipArmor(OutgoingItem, EquippedBodyData);
			EquippedArmorParts.Remove(FStalkerGameplayTags::ArmorPartTag_Body);
			break;
		}
	default:
		break;
	}

	CalculateTotalArmorEndurance();
}

FActiveGameplayEffectHandle UCharacterArmorComponent::ApplyItemEffectSpec(UItemObject* ItemObject)
{
	if (UArmorObject* ArmorObject = Cast<UArmorObject>(ItemObject))
	{
		if (const UClass* ArmorEffectClass = ArmorObject->GetArmorEffect())
		{
			FGameplayEffectContextHandle Context = GetAbilityComponent()->MakeEffectContext();
			if (Context.IsValid())
			{
				Context.AddSourceObject(ArmorObject);

				auto ArmorEffect = ArmorEffectClass->GetDefaultObject<UGameplayEffect>();
				auto ArmorSpec = FGameplayEffectSpec(ArmorEffect, Context, 1.0f);
				const TMap<FGameplayTag, float>& ArmorModifiers = ArmorObject->GetProtectionModifiers();

				for (const auto& Modifier : ArmorModifiers)
				{
					ArmorSpec.SetSetByCallerMagnitude(Modifier.Key, Modifier.Value);
				}

				FActiveGameplayEffectHandle NewEffectHandle = GetAbilityComponent()->ApplyGameplayEffectSpecToSelf(
					ArmorSpec);
				if (NewEffectHandle.IsValid())
				{
					ActiveItemEffects.Add(ArmorObject, NewEffectHandle);
					return NewEffectHandle;
				}
			}
		}
		
	}
	return FActiveGameplayEffectHandle();
}

bool UCharacterArmorComponent::RemoveItemEffectSpec(UItemObject* ItemObject)
{
	if (FActiveGameplayEffectHandle* EffectHandle = ActiveItemEffects.Find(ItemObject))
	{
		if (EffectHandle && EffectHandle->IsValid())
		{
			ActiveItemEffects.Remove(ItemObject);
			return GetAbilityComponent()->RemoveActiveGameplayEffect(*EffectHandle);
		}
	}
	return false;
}

void UCharacterArmorComponent::ReapplyItemEffectSpec(UItemObject* ItemObject)
{
	if (RemoveItemEffectSpec(ItemObject))
	{
		ApplyItemEffectSpec(ItemObject);
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

void UCharacterArmorComponent::OnRep_TotalArmorData()
{
	OnTotalArmorDataChangedDelegate.Broadcast(TotalArmorData);
}
