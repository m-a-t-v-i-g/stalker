// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "AbilitySet.h"
#include "AbilitySystemComponent.h"
#include "CharacterStateComponent.h"
#include "ItemActor.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponObject.h"

UCharacterWeaponComponent::UCharacterWeaponComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, LeftHandItemData,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, RightHandItemData,	COND_OwnerOnly);
	
	DOREPLIFETIME(ThisClass, LeftHandItemActor);
	DOREPLIFETIME(ThisClass, RightHandItemActor);
}

void UCharacterWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetCharacter())
	{
		return;
	}

	TickItemAtHand(DeltaTime);
}

void UCharacterWeaponComponent::OnCharacterDead()
{
	Super::OnCharacterDead();
}

void UCharacterWeaponComponent::TickItemAtHand(float DeltaTime)
{
	if (!IsRightItemObjectValid())
	{
		return;
	}

	if (UItemObject* ItemObject = GetItemObjectAtRightHand())
	{
		ItemObject->Tick(DeltaTime);
	}
}

void UCharacterWeaponComponent::ToggleSlot(uint8 SlotIndex)
{
	if (OutfitSlots.IsValidIndex(SlotIndex))
	{
		ServerToggleSlot(SlotIndex);
	}
}

void UCharacterWeaponComponent::ServerToggleSlot_Implementation(int8 SlotIndex)
{
	const FOutfitSlot* SlotPtr = &OutfitSlots[SlotIndex];
	if (!SlotPtr)
	{
		return;
	}

	if (!SlotPtr->IsArmed())
	{
		return;
	}
	
	ShowOrHideItem(SlotPtr->ArmedObject);
}

void UCharacterWeaponComponent::StartAiming()
{
	OnAimingStartDelegate.Broadcast();

	if (GetOwner())
	{
		if (auto SpringArm = GetOwner()->GetComponentByClass<USpringArmComponent>())
		{
			TargetArmLength = SpringArm->TargetArmLength;
			SpringArm->TargetArmLength = TargetArmLength - 75.0f;
		}
	}
}

void UCharacterWeaponComponent::StopAiming()
{
	OnAimingStopDelegate.Broadcast();
	
	if (GetOwner())
	{
		if (auto SpringArm = GetOwner()->GetComponentByClass<USpringArmComponent>())
		{
			SpringArm->TargetArmLength = TargetArmLength;
		}
	}
}

const FHandItemBehavior* UCharacterWeaponComponent::GetHandItemBehavior(const FName& ItemScriptName) const
{
	if (GetItemBehaviorSet())
	{
		if (const FHandItemBehavior* Behavior = GetItemBehaviorSet()->HandItems.Find(ItemScriptName))
		{
			return Behavior;
		}
	}
	return nullptr;
}

UItemObject* UCharacterWeaponComponent::GetItemObjectAtLeftHand() const
{
	return LeftHandItemData.ItemObject;
}

UItemObject* UCharacterWeaponComponent::GetItemObjectAtRightHand() const
{
	return RightHandItemData.ItemObject;
}

AItemActor* UCharacterWeaponComponent::GetItemActorAtLeftHand() const
{
	return LeftHandItemActor;
}

AItemActor* UCharacterWeaponComponent::GetItemActorAtRightHand() const
{
	return RightHandItemActor;
}

bool UCharacterWeaponComponent::IsLeftItemObjectValid() const
{
	return IsValid(LeftHandItemData.ItemObject);
}

bool UCharacterWeaponComponent::IsRightItemObjectValid() const
{
	return IsValid(RightHandItemData.ItemObject);
}

bool UCharacterWeaponComponent::IsLeftItemActorValid() const
{
	return IsValid(LeftHandItemActor);
}

bool UCharacterWeaponComponent::IsRightItemActorValid() const
{
	return IsValid(RightHandItemActor);
}

void UCharacterWeaponComponent::SetupOutfitComponent(AStalkerCharacter* InCharacter)
{
	Super::SetupOutfitComponent(InCharacter);

	if (GetCharacter())
	{
		GetCharacter()->ToggleSlotDelegate.AddUObject(this, &UCharacterWeaponComponent::ToggleSlot);
	}
}

void UCharacterWeaponComponent::OnEquipSlot(const FString& SlotName, UItemObject* InItem)
{
	ShowOrHideItem(InItem);
}

void UCharacterWeaponComponent::OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem)
{
	if (LeftHandItemData.ItemObject == PrevItem || RightHandItemData.ItemObject == PrevItem)
	{
		ShowOrHideItem(PrevItem);
	}
}

void UCharacterWeaponComponent::ShowOrHideItem(UItemObject* InItem)
{
	if (!IsValid(InItem))
	{
		return;
	}

	const FHandItemBehavior* InItemBeh = GetHandItemBehavior(InItem->GetScriptName());
	if (!InItemBeh)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *InItem->GetScriptName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}

	ECharacterOverlayState TargetOverlay = ECharacterOverlayState::Default;

	const UItemObject* LeftItem = GetItemObjectAtLeftHand();
	const UItemObject* RightItem = GetItemObjectAtRightHand();

	switch (InItemBeh->OccupiedHand)
	{
	case EOccupiedHand::Left:
		{
			if (LeftItem != InItem)
			{
				if (IsLeftItemObjectValid())
				{
					DisarmHand(LeftHandItemData, LeftHandItemActor);
				}

				if (IsRightItemObjectValid())
				{
					if (const FHandItemBehavior* RightItemBeh = GetHandItemBehavior(RightItem->GetScriptName()))
					{
						switch (RightItemBeh->OccupiedHand)
						{
						case EOccupiedHand::Right:
							TargetOverlay = ECharacterOverlayState::LeftAndRightHand;
							break;
						case EOccupiedHand::Both:
							TargetOverlay = ECharacterOverlayState::OnlyLeftHand;
							DisarmHand(RightHandItemData, RightHandItemActor);
							break;
						default: break;
						}
					}
				}

				ArmHand(LeftHandItemData, LeftHandItemActor, FCharacterSocketName::LeftHandSocket, InItem);
			}
			else
			{
				if (IsRightItemObjectValid())
				{
					TargetOverlay = ECharacterOverlayState::OnlyRightHand;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::Default;
				}
				
				DisarmHand(LeftHandItemData, LeftHandItemActor);
			}
			break;
		}
	case EOccupiedHand::Right:
		{
			if (RightItem != InItem)
			{
				if (IsLeftItemObjectValid())
				{
					TargetOverlay = ECharacterOverlayState::LeftAndRightHand;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::OnlyRightHand;
				}
				
				if (IsRightItemObjectValid())
				{
					DisarmHand(RightHandItemData, RightHandItemActor);
				}
				
				ArmHand(RightHandItemData, RightHandItemActor, FCharacterSocketName::RightHandSocket, InItem);
			}
			else
			{
				if (!IsLeftItemObjectValid())
				{
					TargetOverlay = ECharacterOverlayState::Default;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::OnlyLeftHand;
				}
				
				DisarmHand(RightHandItemData, RightHandItemActor);
			}
			break;
		}
	case EOccupiedHand::Both:
		{
			if (RightItem != InItem)
			{
				if (IsLeftItemObjectValid())
				{
					DisarmHand(LeftHandItemData, LeftHandItemActor);
				}

				if (IsRightItemObjectValid())
				{
					DisarmHand(RightHandItemData, RightHandItemActor);
				}

				TargetOverlay = ECharacterOverlayState::BothHands;
				ArmHand(RightHandItemData, RightHandItemActor, FCharacterSocketName::RightHandSocket, InItem);
			}
			else
			{
				TargetOverlay = ECharacterOverlayState::Default;
				DisarmHand(RightHandItemData, RightHandItemActor);
			}
			break;
		}
	default: break;
	}

	GetStateComponent()->SetOverlayState(TargetOverlay, true);
}

void UCharacterWeaponComponent::ArmHand(FEquippedHandEntry& HandedItemData, AItemActor*& ReplicatedItemActor,
                                        const FName& SocketName, UItemObject* ItemObject)
{
	check(ItemObject);

	if (const FHandItemBehavior* HandItemBeh = GetHandItemBehavior(ItemObject->GetScriptName()))
	{
		HandedItemData.ItemObject = ItemObject;
		ReplicatedItemActor = SpawnItemAtHand(GetCharacter()->GetMesh(), ItemObject, SocketName);
		
		check(HandedItemData.ItemObject);

		HandedItemData.AbilitySet = HandItemBeh->AbilitySet;
		HandedItemData.AbilitySet->GiveToAbilitySystem(GetAbilityComponent(), HandedItemData.Abilities, ItemObject);
		HandedItemData.WeaponBehavior = HandItemBeh;

		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(ItemObject))
		{
			BindWeaponObject(WeaponObject);
		}
	}
}

void UCharacterWeaponComponent::DisarmHand(FEquippedHandEntry& HandedItemData, AItemActor*& ReplicatedItemActor)
{
	if (HandedItemData.IsValid() && ReplicatedItemActor)
	{
		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(HandedItemData.ItemObject))
		{
			UnbindWeaponObject(WeaponObject);
		}

		for (const FGameplayAbilitySpecHandle& AbilityHandle : HandedItemData.Abilities)
		{
			GetAbilityComponent()->ClearAbility(AbilityHandle);
		}

		HandedItemData.ItemObject->UnbindItemActor();
		HandedItemData.Clear();
		ReplicatedItemActor->Destroy();
	}
}

void UCharacterWeaponComponent::AddWeaponRecoil()
{
	if (AStalkerCharacter* Character = GetCharacter())
	{
		if (Character->IsLocallyControlled())
		{
			if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(RightHandItemData.ItemObject))
			{
				float RecoilAngle = WeaponObject->GetRecoilAngle();
				float RecoilMultiplier = WeaponObject->GetRecoilMultiplier();
				float ResultRecoil = -RecoilAngle * RecoilMultiplier;
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Recoil angle: %f"), ResultRecoil), true,
				                                  false, FLinearColor::White);
				
				Character->AddControllerPitchInput(ResultRecoil);
			}
		}
	}
}

void UCharacterWeaponComponent::OnWeaponFireStart()
{
	AddWeaponRecoil();
	OnFireStartDelegate.Broadcast();
}

void UCharacterWeaponComponent::OnWeaponFireStop()
{
	OnFireStopDelegate.Broadcast();
}

bool UCharacterWeaponComponent::BindWeaponObject(UWeaponObject* WeaponObject)
{
	if (IsValid(WeaponObject))
	{
		WeaponObject->MakeEquipped();
		WeaponObject->OnFireStartDelegate.AddUObject(this, &UCharacterWeaponComponent::OnWeaponFireStart);
		WeaponObject->OnFireStopDelegate.AddUObject(this, &UCharacterWeaponComponent::OnWeaponFireStop);
		return true;
	}
	return false;
}

bool UCharacterWeaponComponent::UnbindWeaponObject(UWeaponObject* WeaponObject)
{
	if (IsValid(WeaponObject))
	{
		WeaponObject->MakeUnequipped();
		WeaponObject->OnFireStartDelegate.RemoveAll(this);
		WeaponObject->OnFireStopDelegate.RemoveAll(this);
		return true;
	}
	return false;
}

AItemActor* UCharacterWeaponComponent::SpawnItemAtHand(USceneComponent* AttachmentComponent, UItemObject* ItemObject,
                                                       FName SocketName)
{
	if (AttachmentComponent->DoesSocketExist(SocketName))
	{
		if (ItemObject && ItemObject->GetActorClass())
		{
			FTransform SpawnTransform = AttachmentComponent->GetSocketTransform(SocketName);
			FAttachmentTransformRules AttachmentRules{EAttachmentRule::SnapToTarget, true};

			auto SpawnedItem = GetWorld()->SpawnActorDeferred<AItemActor>(ItemObject->GetActorClass(), SpawnTransform,
			                                                              GetCharacter(), GetCharacter(),
			                                                              ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (SpawnedItem)
			{
				ItemObject->BindItemActor(SpawnedItem);
				SpawnedItem->SetEquipped();
				SpawnedItem->AttachToComponent(AttachmentComponent, AttachmentRules, SocketName);
				SpawnedItem->FinishSpawning(SpawnTransform);
				return SpawnedItem;
			}
		}
	}
	return nullptr;
}

void UCharacterWeaponComponent::OnRep_RightHandItemData(const FEquippedHandEntry& PrevHandEntry)
{
	if (RightHandItemData.IsValid())
	{
		UItemObject* ItemObject = RightHandItemData.ItemObject;
		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(ItemObject))
		{
			BindWeaponObject(WeaponObject);
		}
	}
	else
	{
		UItemObject* ItemObject = PrevHandEntry.ItemObject;
		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(ItemObject))
		{
			UnbindWeaponObject(WeaponObject);
		}
	}
}

void UCharacterWeaponComponent::OnRep_RightHandItemActor(AItemActor* PrevItemActor)
{
	
}
