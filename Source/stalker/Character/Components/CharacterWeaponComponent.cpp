// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "AbilitySet.h"
#include "AbilitySystemComponent.h"
#include "CharacterStateComponent.h"
#include "ItemActor.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
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
	
	DOREPLIFETIME_CONDITION(ThisClass, LeftHandItemActor,	COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, RightHandItemActor,	COND_SimulatedOnly);
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
	if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(GetItemObjectAtRightHand()))
	{
		WeaponObject->Tick(DeltaTime);
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
	OnAimingStart.Broadcast();
}

void UCharacterWeaponComponent::StopAiming()
{
	OnAimingStop.Broadcast();
}

float UCharacterWeaponComponent::CalculateSpreadMultiplierForWeapon(float DeltaSeconds) const
{
	const float PawnSpeed = GetCharacter()->GetVelocity().Size();
	const float MovementTargetValue = FMath::GetMappedRangeValueClamped(FVector2D(20.0f, 20.0f + 20.0f),
	                                                                    FVector2D(0.9f, 1.0f), PawnSpeed);
	return FMath::FInterpTo(StandingStillMultiplier, MovementTargetValue, DeltaSeconds, 5.0f);
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
	if (GetCharacter()->HasAuthority() || GetCharacter()->IsLocallyControlled())
	{
		return LeftHandItemData.ItemActor;
	}
	return LeftHandItemActor;
}

AItemActor* UCharacterWeaponComponent::GetItemActorAtRightHand() const
{
	if (GetCharacter()->HasAuthority() || GetCharacter()->IsLocallyControlled())
	{
		return RightHandItemData.ItemActor;
	}
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
	if (GetCharacter()->HasAuthority() || GetCharacter()->IsLocallyControlled())
	{
		return IsValid(LeftHandItemData.ItemActor);
	}
	return IsValid(LeftHandItemActor);
}

bool UCharacterWeaponComponent::IsRightItemActorValid() const
{
	if (GetCharacter()->HasAuthority() || GetCharacter()->IsLocallyControlled())
	{
		return IsValid(RightHandItemData.ItemActor);
	}
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
				if (!IsLeftItemObjectValid())
				{
					TargetOverlay = ECharacterOverlayState::OnlyRightHand;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::LeftAndRightHand;
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
	if (HandedItemData.IsValid())
	{
		DisarmHand(HandedItemData, ReplicatedItemActor);
	}

	HandedItemData.ItemObject = ItemObject;
	HandedItemData.ItemActor = SpawnWeapon(GetCharacter()->GetMesh(), ItemObject, SocketName);

	check(HandedItemData.ItemObject);
	check(HandedItemData.ItemActor);

	if (const FHandItemBehavior* HandItemBeh = GetHandItemBehavior(HandedItemData.ItemObject->GetScriptName()))
	{
		HandedItemData.AbilitySet = HandItemBeh->AbilitySet;
		HandedItemData.AbilitySet->GiveToAbilitySystem(GetAbilityComponent(), HandedItemData.Abilities, ItemObject);
		HandedItemData.WeaponBehavior = HandItemBeh;

		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(ItemObject))
		{
			WeaponObject->OnAttackStart.AddUObject(this, &UCharacterWeaponComponent::OnAttackStart);
			WeaponObject->OnAttackStop.AddUObject(this, &UCharacterWeaponComponent::OnAttackStop);
		}
	}
	
	ReplicatedItemActor = HandedItemData.ItemActor;
	
	if (!IsValid(HandedItemData.ItemActor))
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
	}
}

void UCharacterWeaponComponent::DisarmHand(FEquippedHandEntry& HandedItemData, AItemActor*& ReplicatedItemActor)
{
	if (HandedItemData.IsValid() && ReplicatedItemActor)
	{
		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(HandedItemData.ItemObject))
		{
			WeaponObject->CancelAllActions();
			WeaponObject->OnAttackStart.RemoveAll(this);
			WeaponObject->OnAttackStop.RemoveAll(this);
		}
		
		for (const FGameplayAbilitySpecHandle& AbilityHandle : HandedItemData.Abilities)
		{
			GetAbilityComponent()->ClearAbility(AbilityHandle);
		}
		
		HandedItemData.ItemObject->UnbindItemActor();
		HandedItemData.ItemActor->Destroy();
		HandedItemData.Clear();
		ReplicatedItemActor = nullptr;
	}
}

void UCharacterWeaponComponent::OnAttackStart()
{
	OnFireStart.Broadcast();
}

void UCharacterWeaponComponent::OnAttackStop()
{
	OnFireStop.Broadcast();
}

AItemActor* UCharacterWeaponComponent::SpawnWeapon(USceneComponent* AttachmentComponent, UItemObject* ItemObject,
                                                   FName SocketName)
{
	if (AttachmentComponent->DoesSocketExist(SocketName))
	{
		if (ItemObject && ItemObject->GetActorClass())
		{
			FTransform SpawnTransform = AttachmentComponent->GetSocketTransform(SocketName);
			FAttachmentTransformRules AttachmentRules{EAttachmentRule::SnapToTarget, true};

			auto SpawnedWeapon = GetWorld()->SpawnActorDeferred<AItemActor>(ItemObject->GetActorClass(), SpawnTransform,
			                                                                GetCharacter(), GetCharacter(),
			                                                                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (SpawnedWeapon)
			{
				ItemObject->SetInstigator(this);
				ItemObject->BindItemActor(SpawnedWeapon);
				SpawnedWeapon->SetEquipped();
				SpawnedWeapon->AttachToComponent(AttachmentComponent, AttachmentRules, SocketName);
				SpawnedWeapon->FinishSpawning(SpawnTransform);
				return SpawnedWeapon;
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
			WeaponObject->OnAttackStart.AddUObject(this, &UCharacterWeaponComponent::OnAttackStart);
			WeaponObject->OnAttackStop.AddUObject(this, &UCharacterWeaponComponent::OnAttackStop);
		}
	}
	else
	{
		UItemObject* ItemObject = PrevHandEntry.ItemObject;
		
		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(ItemObject))
		{
			WeaponObject->CancelAllActions();
			WeaponObject->OnAttackStart.RemoveAll(this);
			WeaponObject->OnAttackStop.RemoveAll(this);
		}
	}
}
