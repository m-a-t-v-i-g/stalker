// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"

#include "AbilitySet.h"
#include "AbilitySystemComponent.h"
#include "InventoryComponent.h"
#include "ItemActor.h"
#include "ItemObject.h"
#include "StalkerCharacter.h"
#include "Ammo/AmmoObject.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponObject.h"

UCharacterWeaponComponent::UCharacterWeaponComponent()
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
	
	/* TODO:
	if (ReloadingData.bInProgress && !CharacterRef->CheckReloadAbility())
	{
		CancelReloadWeapon();
	}
	*/
}

void UCharacterWeaponComponent::OnCharacterDead()
{
	Super::OnCharacterDead();
}

void UCharacterWeaponComponent::ToggleSlot(int8 SlotIndex)
{
	ServerToggleSlot(SlotIndex);
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
	
	EquipOrUnequipSlot(SlotPtr->GetSlotName(), SlotPtr->ArmedObject);
}

void UCharacterWeaponComponent::StartFire()
{
	if (!RightHandItemData.IsValid())
	{
		return;
	}

	if (auto RightWeapon = GetItemObjectAtRightHand<UWeaponObject>())
	{
		RightWeapon->StartAttack();
	}
}

void UCharacterWeaponComponent::StopFire()
{
	if (!RightHandItemData.IsValid())
	{
		return;
	}
	
	if (auto RightWeapon = GetItemObjectAtRightHand<UWeaponObject>())
	{
		RightWeapon->StopAttack();
	}
}

void UCharacterWeaponComponent::StartAiming()
{
	OnAimingStart.Broadcast();
}

void UCharacterWeaponComponent::StopAiming()
{
	OnAimingStop.Broadcast();
}

void UCharacterWeaponComponent::StartReloadWeapon()
{
	auto RightWeapon = GetItemObjectAtRightHand<UWeaponObject>();
	if (!RightWeapon || RightWeapon->IsMagFull() || !HasAmmoForReload())
	{
		return;
	}
	
	if (IsAutonomousProxy())
	{
		ReloadingData.ReloadTime = RightWeapon->GetReloadTime();
		SetReloadTimer();

		GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, "Client: Reloading...");
	}

	if (IsAuthority())
	{
		UAmmoObject* Ammo = GetAmmoForReload(RightWeapon->GetCurrentAmmoClass());
		check(Ammo);

		int AmmoCount = Ammo->GetAmount();
		int RequiredCount = RightWeapon->CalculateRequiredAmmoCount();
		int AvailableCount = FMath::Min(RequiredCount, AmmoCount);

		ReloadingData = FReloadingData(RightWeapon, Ammo, AvailableCount, RightWeapon->GetReloadTime(), true);

		SetReloadTimer();
		MulticastStartReloadWeapon(ReloadingData.ReloadTime);

		GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Red, "Server: Reloading...");
	}
}

void UCharacterWeaponComponent::MulticastStartReloadWeapon_Implementation(float ReloadTime)
{
	if (IsSimulatedProxy())
	{
		ReloadingData.ReloadTime = ReloadTime;
		SetReloadTimer();
		
		GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Red, "Simulated: Reloading...");
	}
}

void UCharacterWeaponComponent::CompleteReloadWeapon()
{
	if (IsAuthority())
	{
		if (!ReloadingData.IsValid())
		{
			return;
		}

		GetCharacterInventory()->SubtractOrRemoveItem(ReloadingData.AmmoObject, ReloadingData.RequiredAmmoCount);
		ReloadingData.WeaponObject->IncreaseAmmo(ReloadingData.AmmoObject, ReloadingData.RequiredAmmoCount);
	}
	
	ClearReloadingData(true);

	UKismetSystemLibrary::PrintString(this, FString("Reload completed!"), true, false, FLinearColor::Green);
}

void UCharacterWeaponComponent::CancelReloadWeapon()
{
	ClearReloadingData(false);
	
	UKismetSystemLibrary::PrintString(this, FString("Reload cancelled..."), true, false, FLinearColor::Red);
}

void UCharacterWeaponComponent::SetReloadTimer()
{
	FTimerDelegate ReloadDelegate;
	ReloadDelegate.BindLambda([&, this]
	{
		CompleteReloadWeapon();
	});
	
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, ReloadDelegate, ReloadingData.ReloadTime, false);
	OnReloadStart.Broadcast(ReloadingData.ReloadTime);

	UKismetSystemLibrary::PrintString(this, FString("Reload started..."), true, false);
}

void UCharacterWeaponComponent::ClearReloadingData(bool bWasSuccessful)
{
	OnReloadStop.Broadcast(bWasSuccessful);
	ReloadingData.Clear();
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
}

bool UCharacterWeaponComponent::HasAmmoForReload() const
{
	if (auto RightItem = GetItemObjectAtRightHand<UWeaponObject>())
	{
		if (UAmmoObject* Ammo = GetAmmoForReload(RightItem->GetCurrentAmmoClass()))
		{
			return Ammo->GetAmount() > 0;
		}
	}
	return false;
}

UAmmoObject* UCharacterWeaponComponent::GetAmmoForReload(const UAmmoDefinition* DesiredAmmo) const
{
	if (auto RightItem = GetItemObjectAtRightHand<UWeaponObject>())
	{
		UAmmoObject* ResultAmmo;

		if (DesiredAmmo)
		{
			ResultAmmo = Cast<UAmmoObject>(GetCharacterInventory()->FindItemByDefinition(DesiredAmmo));
			if (ResultAmmo)
			{
				return ResultAmmo;
			}
		}

		for (const UAmmoDefinition* AmmoClass : RightItem->GetAmmoClasses())
		{
			ResultAmmo = Cast<UAmmoObject>(GetCharacterInventory()->FindItemByDefinition(AmmoClass));
			if (ResultAmmo)
			{
				return ResultAmmo;
			}
		}
	}
	return nullptr;
}

const FWeaponBehavior* UCharacterWeaponComponent::GetWeaponBehavior(const FName& ItemScriptName) const
{
	if (ItemBehaviorConfig)
	{
		if (const FWeaponBehavior* Behavior = ItemBehaviorConfig->Weapons.Find(ItemScriptName))
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
	return LeftHandItemData.ItemActor;
}

AItemActor* UCharacterWeaponComponent::GetItemActorAtRightHand() const
{
	return RightHandItemData.ItemActor;
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
	return IsValid(LeftHandItemData.ItemActor);
}

bool UCharacterWeaponComponent::IsRightItemActorValid() const
{
	return IsValid(RightHandItemData.ItemActor);
}

void UCharacterWeaponComponent::OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem)
{
	EquipOrUnequipSlot(SlotName, IncomingItem);
}

void UCharacterWeaponComponent::OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem)
{
	const FOutfitSlot* SlotPtr = FindOutfitSlot(SlotName);
	if (!SlotPtr)
	{
		return;
	}

	if (!SlotPtr->IsArmed())
	{
		return;
	}

	UItemObject* SlotObject = SlotPtr->ArmedObject;

	if (LeftHandItemData.ItemObject == SlotObject || RightHandItemData.ItemObject == SlotObject)
	{
		EquipOrUnequipSlot(SlotName, SlotObject);
	}
}

void UCharacterWeaponComponent::EquipOrUnequipSlot(const FString& SlotName, UItemObject* IncomingItem)
{
	if (!IncomingItem || SlotName.IsEmpty())
	{
		return;
	}

	const FWeaponBehavior* WeaponBehConfig = GetWeaponBehavior(IncomingItem->GetScriptName());
	if (!WeaponBehConfig)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *IncomingItem->GetScriptName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}

	ECharacterOverlayState TargetOverlay = ECharacterOverlayState::Default;
	
	const UItemObject* LeftItem = GetItemObjectAtLeftHand();
	const UItemObject* RightItem = GetItemObjectAtRightHand();
	
	const FWeaponBehavior* RightWeaponBeh = nullptr;
	if (RightItem)
	{
		RightWeaponBeh = GetWeaponBehavior(RightItem->GetScriptName());
	}

	switch (WeaponBehConfig->OccupiedHand)
	{
	case EOccupiedHand::Left:
		{
			if (LeftItem != IncomingItem)
			{
				if (IsRightItemObjectValid())
				{
					if (RightWeaponBeh)
					{
						switch (RightWeaponBeh->OccupiedHand)
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
				ArmHand(LeftHandItemData, LeftHandItemActor, FCharacterSocketName::NAME_LeftHand, IncomingItem);
			}
			else
			{
				if (IsRightItemObjectValid())
				{
					if (!RightWeaponBeh)
					{
						return;
					}
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
			if (RightItem != IncomingItem)
			{
				if (!IsLeftItemObjectValid())
				{
					TargetOverlay = ECharacterOverlayState::OnlyRightHand;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::LeftAndRightHand;
				}
				ArmHand(RightHandItemData, RightHandItemActor, FCharacterSocketName::NAME_RightHand, IncomingItem);
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
			if (RightItem != IncomingItem)
			{
				if (IsLeftItemObjectValid())
				{
					DisarmHand(LeftHandItemData, LeftHandItemActor);
				}

				TargetOverlay = ECharacterOverlayState::BothHands;
				ArmHand(RightHandItemData, RightHandItemActor, FCharacterSocketName::NAME_RightHand, IncomingItem);
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
	
	OnOverlayChanged.Broadcast(TargetOverlay);
}

void UCharacterWeaponComponent::ArmHand(FEquippedWeaponData& HandedItemData, AItemActor*& ReplicatedItemActor,
                                        const FName& SocketName, UItemObject* ItemObject)
{
	if (HandedItemData.IsValid())
	{
		DisarmHand(HandedItemData, ReplicatedItemActor);
	}

	HandedItemData.ItemObject = ItemObject;
	HandedItemData.ItemActor = SpawnWeapon(GetCharacter()->GetMesh(), ItemObject, SocketName);

	if (const FWeaponBehavior* WeaponBeh = GetWeaponBehavior(HandedItemData.ItemObject->GetScriptName()))
	{
		HandedItemData.WeaponBehavior = *WeaponBeh;
		HandedItemData.AbilitySet = WeaponBeh->AbilitySet;
		HandedItemData.AbilitySet->GiveToAbilitySystem(GetAbilityComponent(), HandedItemData.Abilities, ItemObject);

		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(ItemObject))
		{
			WeaponObject->OnAttackStart.AddDynamic(this, &UCharacterWeaponComponent::OnAttackStart);
			WeaponObject->OnAttackStop.AddDynamic(this, &UCharacterWeaponComponent::OnAttackStop);
		}
	}
	
	ReplicatedItemActor = HandedItemData.ItemActor;
	
	if (!IsValid(HandedItemData.ItemActor))
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
	}
}

void UCharacterWeaponComponent::DisarmHand(FEquippedWeaponData& HandedItemData, AItemActor*& ReplicatedItemActor)
{
	if (HandedItemData.IsValid() && ReplicatedItemActor)
	{
		if (UWeaponObject* WeaponObject = Cast<UWeaponObject>(HandedItemData.ItemObject))
		{
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
                                                   FName SocketName) const
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
