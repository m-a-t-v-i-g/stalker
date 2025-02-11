// Fill out your copyright notice in the Description page of Project Settings.

#include "OutfitComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EquipmentComponent.h"
#include "EquipmentSlot.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UOutfitComponent::UOutfitComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), OutfitList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	
	SetIsReplicatedByDefault(true);
}

void UOutfitComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OutfitList);
}

void UOutfitComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner())
	{
		SetupOutfitComponent();
	}
}

void UOutfitComponent::SetupOutfitComponent()
{
	AActor* Owner = GetOwner();
	check(Owner);
	
	EquipmentComponentRef = Owner->GetComponentByClass<UEquipmentComponent>();
	ensureMsgf(EquipmentComponentRef != nullptr, TEXT("Make sure the owner has the Equipment Component!"));
	
	if (HasAuthority())
	{
		if (EquipmentComponentRef)
		{
			TArray<UEquipmentSlot*> EquipmentSlots = EquipmentComponentRef->GetEquipmentSlots();
			if (!EquipmentSlots.IsEmpty())
			{
				for (UEquipmentSlot* EquipmentSlot : EquipmentSlots)
				{
					if (!IsValid(EquipmentSlot) || EquipmentSlot->GetSlotName().IsEmpty())
					{
						continue;
					}

					OnEquipmentSlotChange(FEquipmentSlotChangeData(EquipmentSlot->GetSlotName(),
																   EquipmentSlot->GetBoundObject(),
																   EquipmentSlot->IsEquipped()));
					EquipmentSlot->OnSlotDataChange.AddUObject(this, &UOutfitComponent::OnEquipmentSlotChange);
				}
			}
		}
	}
}

void UOutfitComponent::OnEquipmentSlotChange(const FEquipmentSlotChangeData& SlotData)
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
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s: %s slot equipped!"), *GetName(), *SlotName),
										  true, false);
	}
	else
	{
		DisarmSlot(SlotName, SlotItem);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s: %s slot unequipped!"), *GetName(), *SlotName),
										  true, false);
	}
}

UItemObject* UOutfitComponent::ArmSlot(const FString& SlotName, UItemObject* ItemObject)
{
	UItemObject* ArmItem = nullptr;
	if (ItemObject)
	{
		ArmItem = OutfitList.AddEntry(SlotName, ItemObject);
		if (ArmItem)
		{
			OnEquipSlot(SlotName, ArmItem);
			K2_OnEquipSlot(SlotName, ArmItem);
		}
	}
	return ArmItem;
}

void UOutfitComponent::DisarmSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (!SlotName.IsEmpty())
	{
		OnUnequipSlot(SlotName, ItemObject);
		K2_OnUnequipSlot(SlotName, ItemObject);
		OutfitList.RemoveEntry(SlotName);
	}
}

void UOutfitComponent::OnEquipSlot(const FString& SlotName, UItemObject* InItem)
{
}

void UOutfitComponent::OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem)
{
}

bool UOutfitComponent::ApplyItemGameplayEffects(UItemObject* ItemObject)
{
	check(GetOwner());
	
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(GetOwner());
	if (AbilitySystemComponent != nullptr)
	{
		const TArray<TSubclassOf<UGameplayEffect>>& ItemEffects = ItemObject->GetItemEffects();
		if (!ItemEffects.IsEmpty())
		{
			FActiveOutfitGE ActiveItemEffect;
			for (const UClass* ItemEffect : ItemEffects)
			{
				FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
				if (Context.IsValid())
				{
					Context.AddSourceObject(ItemObject);

					auto ItemGE = ItemEffect->GetDefaultObject<UGameplayEffect>();
					auto ItemGESpec = FGameplayEffectSpec(ItemGE, Context, 1.0f);
					ModifyItemEffectSpec(ItemGESpec, ItemObject);

					FActiveGameplayEffectHandle NewEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(ItemGESpec);
					if (NewEffectHandle.IsValid())
					{
						ActiveItemEffect.Handles.Add(NewEffectHandle);
					}
				}
			}

			if (!ActiveItemEffect.Handles.IsEmpty())
			{
				ActiveItemEffects.Add(ItemObject, ActiveItemEffect);
				return true;
			}
		}
	}
	return false;
}

bool UOutfitComponent::RemoveItemGameplayEffects(UItemObject* ItemObject)
{
	check(GetOwner());
	
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(GetOwner());
	bool bResult = false;
	
	if (AbilitySystemComponent != nullptr)
	{
		if (FActiveOutfitGE* ActiveOutfitGE = ActiveItemEffects.Find(ItemObject))
		{
			bResult = true;
			TArray<FActiveGameplayEffectHandle>* Handles = &ActiveOutfitGE->Handles;
			if (Handles && !Handles->IsEmpty())
			{
				for (const FActiveGameplayEffectHandle& Handle : ActiveOutfitGE->Handles)
				{
					if (Handle.IsValid())
					{
						bResult &= AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
					}
				}

				if (bResult)
				{
					ActiveItemEffects.Remove(ItemObject);
				}
			}
		}
	}
	return bResult;
}

void UOutfitComponent::ReapplyItemGameplayEffects(UItemObject* ItemObject)
{
	if (RemoveItemGameplayEffects(ItemObject))
	{
		ApplyItemGameplayEffects(ItemObject);
	}
}

void UOutfitComponent::ModifyItemEffectSpec(FGameplayEffectSpec& Spec, UItemObject* ItemObject)
{
}

const TMap<UItemObject*, FActiveOutfitGE>& UOutfitComponent::GetActiveItemEffects() const
{
	return ActiveItemEffects;
}
