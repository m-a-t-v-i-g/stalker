// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "ItemActor.h"
#include "ItemObject.h"
#include "Ammo/AmmoObject.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/StalkerCharacter.h"
#include "Components/EquipmentSlot.h"
#include "Data/ItemBehaviorConfig.h"
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

	if (!CharacterRef)
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

void UCharacterWeaponComponent::SetupWeaponComponent(AStalkerCharacter* InCharacter)
{
	CharacterRef = InCharacter;

	if (!CharacterRef)
	{
		UE_LOG(LogCharacter, Error, TEXT("Unable to setup the Weapon Component: character ref is null."));
		return;
	}

	InventoryComponentRef = CharacterRef->GetInventoryComponent();
	StateComponentRef = CharacterRef->GetStateComponent();
}

void UCharacterWeaponComponent::InitCharacterInfo(AController* InController)
{
	ControllerRef = InController;
	
	if (!CharacterRef || !ControllerRef)
	{
		UE_LOG(LogCharacter, Error,
		       TEXT(
			       "Unable to setup the Weapon Component for character: one of the references (character or controller) is null."
		       ));
		return;
	}

	if (IsAuthority())
	{
		if (InventoryComponentRef)
		{
			for (uint8 i = 0; i < WeaponSlots.Num(); i++)
			{
				if (WeaponSlots[i].SlotName.IsEmpty())
				{
					continue;
				}

				if (UEquipmentSlot* SlotPtr = InventoryComponentRef->FindEquipmentSlot(WeaponSlots[i].SlotName))
				{
					SlotPtr->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSlotEquipped,
													  WeaponSlots[i].SlotName);
					OnSlotEquipped(FUpdatedSlotData(SlotPtr->GetBoundObject(), SlotPtr->IsEquipped()), WeaponSlots[i].SlotName);
				}
			}
		}
	}
}

bool UCharacterWeaponComponent::CanAttack() const
{
	return Super::CanAttack();
}

bool UCharacterWeaponComponent::IsArmed() const
{
	return RightHandItemActor->IsA<UWeaponObject>();
}

void UCharacterWeaponComponent::ServerToggleSlot_Implementation(int8 SlotIndex)
{
	const FWeaponSlot* SlotPtr = &WeaponSlots[SlotIndex];
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

void UCharacterWeaponComponent::PlayBasicAction()
{
	if (!RightHandItemData.IsValid())
	{
		return;
	}

	switch (RightHandItemData.ItemBehavior.LeftMouseReaction)
	{
	case EMouseButtonReaction::Attack:
		StartFire();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::StopBasicAction()
{
	if (!RightHandItemData.IsValid())
	{
		return;
	}

	switch (RightHandItemData.ItemBehavior.LeftMouseReaction)
	{
	case EMouseButtonReaction::Attack:
		StopFire();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::PlayAlternativeAction()
{
	const UItemObject* RightItem = GetItemObjectAtRightHand();
	
	if (!RightItem || !IsRightItemActorValid())
	{
		return;
	}

	const FItemBehavior* RightItemBeh = WeaponBehaviorConfig->GetItemBehavior(RightItem->GetScriptName());
	if (!RightItemBeh)
	{
		return;
	}

	switch (RightItemBeh->RightMouseReaction)
	{
	case EMouseButtonReaction::Alternative:
		StartAlternative();
		break;
	case EMouseButtonReaction::Aiming:
		StartAiming();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::StopAlternativeAction()
{
	const UItemObject* RightItem = GetItemObjectAtRightHand();
	
	if (!RightItem || !IsRightItemActorValid())
	{
		return;
	}

	const FItemBehavior* RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetScriptName());
	if (!RightItemBeh)
	{
		return;
	}

	switch (RightItemBeh->RightMouseReaction)
	{
	case EMouseButtonReaction::Alternative:
		StopAlternative();
		break;
	case EMouseButtonReaction::Aiming:
		StopAiming();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::StartFire()
{
	if (!RightHandItemData.IsValid())
	{
		return;
	}
	
	if (auto RightWeapon = Cast<UWeaponObject>(GetItemObjectAtRightHand()))
	{
		OnFireStart.Broadcast();
		RightWeapon->StartAttack();
	}

	if (IsAutonomousProxy())
	{
		ServerStartFire();
	}
}

void UCharacterWeaponComponent::ServerStartFire_Implementation()
{
	StartFire();
}

void UCharacterWeaponComponent::StopFire()
{
	if (!RightHandItemData.IsValid())
	{
		return;
	}
	
	if (auto RightWeapon = Cast<UWeaponObject>(GetItemObjectAtRightHand()))
	{
		OnFireStop.Broadcast();
		RightWeapon->StopAttack();
	}
	
	if (IsAutonomousProxy())
	{
		ServerStopFire();
	}
}

void UCharacterWeaponComponent::ServerStopFire_Implementation()
{
	StopFire();
}

void UCharacterWeaponComponent::StartAlternative()
{
}

void UCharacterWeaponComponent::ServerStartAlternative_Implementation()
{
}

void UCharacterWeaponComponent::StopAlternative()
{
}

void UCharacterWeaponComponent::ServerStopAlternative_Implementation()
{
}

void UCharacterWeaponComponent::StartAiming()
{
	OnAimingStart.Broadcast();

	if (IsAutonomousProxy())
	{
		ServerStartAiming();
	}
}

void UCharacterWeaponComponent::ServerStartAiming_Implementation()
{
	StartAiming();
}

void UCharacterWeaponComponent::StopAiming()
{
	OnAimingStop.Broadcast();

	if (IsAutonomousProxy())
	{
		ServerStopAiming();
	}
}

void UCharacterWeaponComponent::ServerStopAiming_Implementation()
{
	StopAiming();
}

void UCharacterWeaponComponent::TryReloadWeapon()
{
	if (IsAutonomousProxy())
	{
		auto RightWeapon = GetItemObjectAtRightHand<UWeaponObject>();
		if (!RightWeapon || RightWeapon->IsMagFull() || !HasAmmoForReload())
		{
			return;
		}

		ReloadingData.ReloadTime = RightWeapon->GetReloadTime();
		SetReloadTimer();

		GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Green, "Client: Reloading...");
	}
	
	ServerTryReloadWeapon();
}

void UCharacterWeaponComponent::ServerTryReloadWeapon_Implementation()
{
	auto RightWeapon = GetItemObjectAtRightHand<UWeaponObject>();
	if (!RightWeapon || RightWeapon->IsMagFull() || !HasAmmoForReload())
	{
		return;
	}

	UAmmoObject* Ammo = GetAmmoForReload(RightWeapon->GetCurrentAmmoClass());
	check(Ammo);

	int AmmoCount = Ammo->GetAmount();
	int RequiredCount = RightWeapon->CalculateRequiredAmmoCount();
	int AvailableCount = FMath::Min(RequiredCount, AmmoCount);

	ReloadingData = FReloadingData(RightWeapon, Ammo, AvailableCount, RightWeapon->GetReloadTime(), true);

	SetReloadTimer();
	MulticastReloadWeapon(ReloadingData.ReloadTime);

	GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Red, "Server: Reloading...");
}

void UCharacterWeaponComponent::MulticastReloadWeapon_Implementation(float ReloadTime)
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

		GetCharacterInventory()->ServerSubtractOrRemoveItem(ReloadingData.AmmoObject, ReloadingData.AmmoCount);
		ReloadingData.WeaponObject->IncreaseAmmo(ReloadingData.AmmoObject, ReloadingData.AmmoCount);
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
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	OnReloadStop.Broadcast(bWasSuccessful);
	ReloadingData.Clear();
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

void UCharacterWeaponComponent::EquipOrUnequipSlot(const FString& SlotName, UItemObject* IncomingItem)
{
	if (!IncomingItem || SlotName.IsEmpty())
	{
		return;
	}

	const FItemBehavior* WeaponConfig = GetItemBehavior(IncomingItem->GetScriptName());
	if (!WeaponConfig)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *IncomingItem->GetScriptName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}

	ECharacterOverlayState TargetOverlay = ECharacterOverlayState::Default;
	
	const UItemObject* LeftItem = GetItemObjectAtLeftHand();
	const UItemObject* RightItem = GetItemObjectAtRightHand();
	
	const FItemBehavior* RightItemBeh = nullptr;
	if (RightItem)
	{
		RightItemBeh = GetItemBehavior(RightItem->GetScriptName());
	}

	switch (WeaponConfig->OccupiedHand)
	{
	case EOccupiedHand::Left:
		{
			if (LeftItem != IncomingItem)
			{
				if (IsRightItemObjectValid())
				{
					if (RightItemBeh)
					{
						switch (RightItemBeh->OccupiedHand)
						{
						case EOccupiedHand::Right:
							TargetOverlay = ECharacterOverlayState::LeftAndRightHand;
							break;
						case EOccupiedHand::Both:
							TargetOverlay = ECharacterOverlayState::OnlyLeftHand;
							DisarmRightHand();
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
					if (!RightItemBeh)
					{
						return;
					}
					TargetOverlay = ECharacterOverlayState::OnlyRightHand;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::Default;
				}
				DisarmLeftHand();
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
				DisarmRightHand();
			}
			break;
		}
	case EOccupiedHand::Both:
		{
			if (RightItem != IncomingItem)
			{
				if (IsLeftItemObjectValid())
				{
					DisarmLeftHand();
				}

				TargetOverlay = ECharacterOverlayState::BothHands;
				ArmHand(RightHandItemData, RightHandItemActor, FCharacterSocketName::NAME_RightHand, IncomingItem);
			}
			else
			{
				TargetOverlay = ECharacterOverlayState::Default;
				DisarmRightHand();
			}
			break;
		}
	default: break;
	}
	
	OnOverlayChanged.Broadcast(TargetOverlay);
}

void UCharacterWeaponComponent::UnEquipSlot(const FString& SlotName)
{
	const FWeaponSlot* SlotPtr = FindWeaponSlot(SlotName);
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

bool UCharacterWeaponComponent::ArmLeftHand(UItemObject* ItemObject)
{
	check(ItemObject);

	if (LeftHandItemData.IsValid())
	{
		DisarmLeftHand();
	}

	LeftHandItemData.ItemObject = ItemObject;
	LeftHandItemData.ItemActor = SpawnWeapon(CharacterRef->GetMesh(), ItemObject, FCharacterSocketName::NAME_LeftHand);
	
	if (const FItemBehavior* ItemBeh = GetItemBehavior(LeftHandItemData.ItemObject->GetScriptName()))
	{
		LeftHandItemData.ItemBehavior = *ItemBeh;
	}
	
	LeftHandItemActor = LeftHandItemData.ItemActor;
	
	if (!IsValid(LeftHandItemData.ItemActor))
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
		return false;
	}
	return true;
}

bool UCharacterWeaponComponent::ArmRightHand(UItemObject* ItemObject)
{
	check(ItemObject);
	
	if (RightHandItemActor)
	{
		DisarmRightHand();
	}

	RightHandItemData.ItemObject = ItemObject;
	RightHandItemData.ItemActor = SpawnWeapon(CharacterRef->GetMesh(), ItemObject, FCharacterSocketName::NAME_RightHand);
	
	if (const FItemBehavior* ItemBeh = GetItemBehavior(RightHandItemData.ItemObject->GetScriptName()))
	{
		RightHandItemData.ItemBehavior = *ItemBeh;
	}
	
	RightHandItemActor = RightHandItemData.ItemActor;
	
	if (!IsValid(RightHandItemData.ItemActor))
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
		return false;
	}
	return true;
}

void UCharacterWeaponComponent::DisarmLeftHand()
{
	if (LeftHandItemData.IsValid() && LeftHandItemActor)
	{
		LeftHandItemData.ItemObject->UnbindItemActor();
		LeftHandItemData.ItemActor->Destroy();
		LeftHandItemData.Clear();
		LeftHandItemActor = nullptr;
	}
}

void UCharacterWeaponComponent::DisarmRightHand()
{
	if (RightHandItemData.IsValid() && RightHandItemActor)
	{
		RightHandItemData.ItemObject->UnbindItemActor();
		RightHandItemData.ItemActor->Destroy();
		RightHandItemData.Clear();
		RightHandItemActor = nullptr;
	}
}

void UCharacterWeaponComponent::ArmHand(FHandedItemData& HandedItemData, AItemActor*& ReplicatedItemActor,
                                        const FName& SocketName, UItemObject* ItemObject)
{
	if (HandedItemData.IsValid())
	{
		DisarmHand(HandedItemData, ReplicatedItemActor);
	}

	HandedItemData.ItemObject = ItemObject;
	HandedItemData.ItemActor = SpawnWeapon(CharacterRef->GetMesh(), ItemObject, SocketName);
	
	if (const FItemBehavior* ItemBeh = GetItemBehavior(HandedItemData.ItemObject->GetScriptName()))
	{
		HandedItemData.ItemBehavior = *ItemBeh;
	}
	
	ReplicatedItemActor = HandedItemData.ItemActor;
	
	if (!IsValid(HandedItemData.ItemActor))
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
	}
}

void UCharacterWeaponComponent::DisarmHand(FHandedItemData& HandedItemData, AItemActor*& ReplicatedItemActor)
{
	if (HandedItemData.IsValid() && ReplicatedItemActor)
	{
		HandedItemData.ItemObject->UnbindItemActor();
		HandedItemData.ItemActor->Destroy();
		HandedItemData.Clear();
		ReplicatedItemActor = nullptr;
	}
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
			                                                                CharacterRef, CharacterRef,
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

void UCharacterWeaponComponent::OnSlotEquipped(const FUpdatedSlotData& SlotData, FString SlotName)
{
	if (!IsValid(SlotData.SlotItem) || SlotName.IsEmpty())
	{
		return;
	}

	if (SlotData.bIsEquipped)
	{	
		ArmSlot(SlotName, SlotData.SlotItem);
		EquipOrUnequipSlot(SlotName, SlotData.SlotItem);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot equipped!"), *SlotName), true, false);
	}
	else
	{
		UnEquipSlot(SlotName);
		DisarmSlot(SlotName);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot unequipped!"), *SlotName), true, false);
	}
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

const FItemBehavior* UCharacterWeaponComponent::GetItemBehavior(const FName& ItemScriptName) const
{
	if (WeaponBehaviorConfig)
	{
		if (const FItemBehavior* Behavior = WeaponBehaviorConfig->ItemsMap.Find(ItemScriptName))
		{
			return Behavior;
		}
	}
	return nullptr;
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

UCharacterInventoryComponent* UCharacterWeaponComponent::GetCharacterInventory() const
{
	if (CharacterRef)
	{
		return CharacterRef->GetInventoryComponent();
	}
	return nullptr;
}
