// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArmorComponent.h"
#include "AbilitySystemComponent.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
#include "Armor/ArmorObject.h"
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

void UCharacterArmorComponent::EquipArmor(UItemObject* ItemObject, FEquippedArmorData& ArmorData)
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

	if (ItemEffects.Contains(ArmorObject))
	{
		return;
	}
	
	const FArmorBehavior* ArmorBehConfig = GetArmorBehavior(ArmorObject->GetScriptName());
	if (!ArmorBehConfig)
	{
		return;
	}

	FActiveGameplayEffectHandle NewEffectHandle = ApplyItemEffectSpec(ArmorObject);
	if (NewEffectHandle.IsValid())
	{
		ItemEffects.Add(ArmorObject, NewEffectHandle);
	}

	ArmorObject->OnEnduranceChangedDelegate.AddUObject(this, &UCharacterArmorComponent::OnEquippedArmorEnduranceChanged,
	                                                   ItemObject);
	ArmorData = FEquippedArmorData(ArmorObject, *ArmorBehConfig);
}

void UCharacterArmorComponent::UnequipArmor(UItemObject* ItemObject, FEquippedArmorData& ArmorData)
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

	if (RemoveItemEffectSpec(ArmorObject))
	{
		ItemEffects.Remove(ArmorObject);
	}

	ArmorObject->OnEnduranceChangedDelegate.RemoveAll(this);
	ArmorData.Clear();
}

void UCharacterArmorComponent::OnEquippedArmorEnduranceChanged(float ItemEndurance, UItemObject* ItemObject)
{
	ReapplyItemEffectSpec(ItemObject);
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
			EquipArmor(IncomingItem, EquippedHelmetData);
			break;
		}
	case EArmorType::Body:
		{
			EquipArmor(IncomingItem, EquippedBodyData);
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
			UnequipArmor(OutgoingItem, EquippedHelmetData);
			break;
		}
	case EArmorType::Body:
		{
			UnequipArmor(OutgoingItem, EquippedBodyData);
			break;
		}
	default:
		break;
	}
}

void UCharacterArmorComponent::OnCharacterDamaged(const FGameplayTag& DamageTag, const FHitResult& HitResult)
{
	FGameplayTag HitBone = HitScanMap.FindChecked(HitResult.BoneName);
	if (HitBone.IsValid())
	{
		if (UItemObject* DamagedArmor = EquippedArmor.FindChecked(HitBone))
		{
			DamagedArmor->SpoilEndurance(DamageTag);
		}
	}
}

FActiveGameplayEffectHandle UCharacterArmorComponent::ApplyItemEffectSpec(UItemObject* ItemObject)
{
	if (UArmorObject* ArmorObject = Cast<UArmorObject>(ItemObject))
	{
		if (UClass* ArmorEffectClass = ArmorObject->GetArmorEffect())
		{
			if (auto ArmorEffect = NewObject<UGameplayEffect>(GetAbilityComponent(), ArmorEffectClass))
			{
				FGameplayEffectContextHandle Context = GetAbilityComponent()->MakeEffectContext();
				if (Context.IsValid())
				{
					Context.AddSourceObject(ArmorObject);
				
					auto ArmorSpec = FGameplayEffectSpec(ArmorEffect, Context, 1.0f);
					const TMap<FGameplayTag, float>& ArmorModifiers = ArmorObject->GetModifiers();

					for (const auto& Modifier : ArmorModifiers)
					{
						ArmorSpec.SetSetByCallerMagnitude(Modifier.Key, Modifier.Value);
					}
				
					FActiveGameplayEffectHandle NewEffectHandle = GetAbilityComponent()->ApplyGameplayEffectSpecToSelf(ArmorSpec);
					if (NewEffectHandle.IsValid())
					{
						return NewEffectHandle;
					}
				}
			}
		}
	}
	return FActiveGameplayEffectHandle();
}

bool UCharacterArmorComponent::RemoveItemEffectSpec(UItemObject* ItemObject)
{
	if (FActiveGameplayEffectHandle* EffectHandle = ItemEffects.Find(ItemObject))
	{
		if (EffectHandle && EffectHandle->IsValid())
		{
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
