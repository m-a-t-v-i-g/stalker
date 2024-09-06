// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "ItemActor.h"
#include "ItemObject.h"
#include "Ammo/AmmoObject.h"
#include "Characters/StalkerCharacter.h"
#include "DataAssets/ItemBehaviorDataAsset.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Inventory/EquipmentSlot.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponActor.h"
#include "Weapons/WeaponObject.h"

UCharacterWeaponComponent::UCharacterWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterWeaponComponent, LeftHandItem);
	DOREPLIFETIME(UCharacterWeaponComponent, RightHandItem);
	DOREPLIFETIME(UCharacterWeaponComponent, bAiming);
}

void UCharacterWeaponComponent::PreInitializeWeapon()
{
	if (!GetOwner()->HasAuthority()) return;
	
	Super::PreInitializeWeapon();
	
	StalkerCharacter = GetOwner<AStalkerCharacter>();
	if (!StalkerCharacter.Get()) return;

	if (auto CharacterInventoryComp = StalkerCharacter->GetComponentByClass<UCharacterInventoryComponent>())
	{
		CharacterInventory = CharacterInventoryComp;
		for (uint8 i = 0; i < WeaponSlotSpecs.Num(); i++)
		{
			if (WeaponSlotSpecs[i].SlotName.IsEmpty()) continue;
			if (auto SlotPtr = CharacterInventory->FindEquipmentSlot(WeaponSlotSpecs[i].SlotName))
			{
				SlotPtr->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSlotEquipped, WeaponSlotSpecs[i].SlotName);
			}
		}
	}
}

void UCharacterWeaponComponent::PostInitializeWeapon()
{
	Super::PostInitializeWeapon();
	
	if (!GetOwner()->HasAuthority()) return;
}

bool UCharacterWeaponComponent::CanAttack() const
{
	return Super::CanAttack();
}

bool UCharacterWeaponComponent::IsArmed() const
{
	return LeftHandItem->IsA<UWeaponObject>() || RightHandItem->IsA<UWeaponObject>();
}

void UCharacterWeaponComponent::ServerToggleSlot_Implementation(int8 SlotIndex)
{
	auto* SlotPtr = &WeaponSlots[SlotIndex];
	if (!SlotPtr) return;

	if (!SlotPtr->IsArmed()) return;
	
	auto SlotObject = SlotPtr->ArmedItemObject;
	EquipOrUnEquipHands(SlotPtr->GetSlotName(), SlotObject);
}

void UCharacterWeaponComponent::PlayBasicAction()
{
	auto RightItem = GetItemAtRightHand();
	if (!RightItem || !IsRightItemActorValid()) return;

	FItemBehavior* RightItemBeh = WeaponBehaviorData->ItemsMap.Find(RightItem->GetItemRowName());
	if (!RightItemBeh)
	{
		return;
	}

	switch (RightItemBeh->LeftMouseReaction)
	{
	case EMouseButtonReaction::Attack:
		StartFire();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::StopBasicAction()
{
	auto RightItem = GetItemAtRightHand();
	if (!RightItem || !IsRightItemActorValid()) return;

	FItemBehavior* RightItemBeh = WeaponBehaviorData->ItemsMap.Find(RightItem->GetItemRowName());
	if (!RightItemBeh)
	{
		return;
	}

	switch (RightItemBeh->LeftMouseReaction)
	{
	case EMouseButtonReaction::Attack:
		StopFire();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::PlayAlternativeAction()
{
	auto RightItem = GetItemAtRightHand();
	if (!RightItem || !IsRightItemActorValid()) return;

	FItemBehavior* RightItemBeh = WeaponBehaviorData->ItemsMap.Find(RightItem->GetItemRowName());
	if (!RightItemBeh)
	{
		return;
	}

	switch (RightItemBeh->RightMouseReaction)
	{
	case EMouseButtonReaction::Aiming:
		StartAiming();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::StopAlternativeAction()
{
	auto RightItem = GetItemAtRightHand();
	if (!RightItem || !IsRightItemActorValid()) return;

	FItemBehavior* RightItemBeh = WeaponBehaviorData->ItemsMap.Find(RightItem->GetItemRowName());
	if (!RightItemBeh)
	{
		return;
	}

	switch (RightItemBeh->RightMouseReaction)
	{
	case EMouseButtonReaction::Aiming:
		StopAiming();
		break;
	default: break;
	}
}

void UCharacterWeaponComponent::StartFire()
{
	auto RightItem = GetActorAtRightHand();
	if (!IsRightItemActorValid()) return;

	if (auto RightWeapon = Cast<AWeaponActor>(RightItem))
	{
		if (!bAiming)
		{
			OnAimingStart.Broadcast();
		}
		RightWeapon->StartAttack();
	}

	if (!GetOwner()->HasAuthority())
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
	auto RightItem = GetActorAtRightHand();
	if (!IsRightItemActorValid()) return;

	if (auto RightWeapon = Cast<AWeaponActor>(RightItem))
	{
		if (!bAiming)
		{
			OnAimingStop.Broadcast();
		}
		RightWeapon->StopAttack();
	}
	
	if (!GetOwner()->HasAuthority())
	{
		ServerStopFire();
	}
}

void UCharacterWeaponComponent::ServerStopFire_Implementation()
{
	StopFire();
}

void UCharacterWeaponComponent::StartAiming()
{
	bAiming = true;
	OnAimingStart.Broadcast();

	if (!GetOwner()->HasAuthority())
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
	bAiming = false;
	OnAimingStop.Broadcast();

	if (!GetOwner()->HasAuthority())
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
	auto RightItem = GetItemAtRightHand<UWeaponObject>();
	auto RightActor = GetActorAtRightHand<AWeaponActor>();
	
	if (!RightItem || !RightActor || RightItem->IsMagFull()) return;

	UAmmoObject* Ammo = nullptr;
	auto Params = RightItem->GetWeaponParams();
	for (auto AmmoClass : Params.AmmoClasses)
	{
		Ammo = Cast<UAmmoObject>(GetCharacterInventory()->FindItemByClass(AmmoClass.Get()));
		if (Ammo) break;
	}

	int AmmoCount = Ammo->GetItemParams().Amount;
	if (!Ammo || AmmoCount <= 0) return;
	
	int RequiredCount = RightItem->CalculateRequiredAmmoCount();
	int AvailableCount = FMath::Min(RequiredCount, AmmoCount);

	OnReloadStart.Broadcast(true);
	SetReloadTimer(RightItem, Ammo, AvailableCount);
}

void UCharacterWeaponComponent::CompleteReloadWeapon(UWeaponObject* WeaponObject, UAmmoObject* AmmoObject, uint16 AmmoCount)
{
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	
	if (!WeaponObject || !AmmoObject) return;

	OnReloadStop.Broadcast(true);
	GetCharacterInventory()->SubtractOrRemoveItem(AmmoObject, AmmoCount);
	WeaponObject->IncreaseAmmo(AmmoCount);

	UKismetSystemLibrary::PrintString(this, FString("Reload completed!"), true, false, FLinearColor::Green);
}

void UCharacterWeaponComponent::CancelReloadWeapon()
{
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	OnReloadStop.Broadcast(false);
	
	UKismetSystemLibrary::PrintString(this, FString("Reload cancelled..."), true, false, FLinearColor::Red);
}

void UCharacterWeaponComponent::SetReloadTimer(UWeaponObject* WeaponObject, UAmmoObject* AmmoObject, uint16 AmmoCount)
{
	float ReloadTime = WeaponObject->GetWeaponParams().ReloadTime;
	FTimerDelegate ReloadDelegate;
	ReloadDelegate.BindLambda([&, this, WeaponObject, AmmoObject, AmmoCount]
	{
		CompleteReloadWeapon(WeaponObject, AmmoObject, AmmoCount);
	});
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, ReloadDelegate, ReloadTime, true);

	UKismetSystemLibrary::PrintString(this, FString("Reload started..."), true, false);
}

void UCharacterWeaponComponent::EquipOrUnEquipHands(const FString& SlotName, UItemObject* ItemObject)
{
	if (!ItemObject) return;
	
	const auto ItemBehavior = WeaponBehaviorData->ItemsMap.Find(ItemObject->GetItemRowName());
	if (!ItemBehavior)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *ItemObject->GetItemRowName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}
	
	ECharacterOverlayState TargetOverlay = ECharacterOverlayState::Default;
	
	auto LeftItem = GetItemAtLeftHand();
	auto RightItem = GetItemAtRightHand();
	
	FItemBehavior* RightItemBeh = nullptr;
	if (RightItem)
	{
		RightItemBeh = WeaponBehaviorData->ItemsMap.Find(RightItem->GetItemRowName());
	}
	
	switch (ItemBehavior->OccupiedHand)
	{
	case ECharacterSlotHand::Left:
		{
			if (LeftItem != ItemObject)
			{
				if (RightItem)
				{
					if (!RightItemBeh) return;
					switch (RightItemBeh->OccupiedHand)
					{
					case ECharacterSlotHand::Right:
						TargetOverlay = RightItemBeh->SecondaryOverlay;
						break;
					case ECharacterSlotHand::Both:
						TargetOverlay = ECharacterOverlayState::Pistol_1H;
						DisarmRightHand();
						break;
					default: break;
					}
				}
				ArmLeftHand(SlotName, ItemObject);
			}
			else
			{
				if (RightItem)
				{
					if (!RightItemBeh) return;
					switch (RightItemBeh->OccupiedHand)
					{
					case ECharacterSlotHand::Right:
						TargetOverlay = RightItemBeh->PrimaryOverlay;
						break;
					default: break;
					}
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::Default;
				}
				DisarmLeftHand();
			}
			break;
		}
	case ECharacterSlotHand::Right:
		{
			TargetOverlay = LeftItem ? ItemBehavior->SecondaryOverlay : ItemBehavior->PrimaryOverlay;
			if (RightItem != ItemObject)
			{
				ArmRightHand(SlotName, ItemObject);
			}
			else
			{
				DisarmRightHand();
				if (!LeftItem)
				{
					TargetOverlay = ECharacterOverlayState::Default;
				}
			}
			break;
		}
	case ECharacterSlotHand::Both:
		{
			TargetOverlay = ItemBehavior->PrimaryOverlay;
			if (RightItem != ItemObject)
			{
				if (LeftItem)
				{
					DisarmLeftHand();
				}
				ArmRightHand(SlotName, ItemObject);
			}
			else
			{
				DisarmRightHand();
				TargetOverlay = ECharacterOverlayState::Default;
			}
			break;
		}
	default: break;
	}
	OnWeaponOverlayChanged.Broadcast(TargetOverlay);
}

void UCharacterWeaponComponent::UnEquipHands(const FString& SlotName)
{
	auto SlotPtr = FindWeaponSlot(SlotName);
	if (!SlotPtr) return;

	if (!SlotPtr->IsArmed()) return;

	auto SlotObject = SlotPtr->ArmedItemObject;
	auto LeftItem = GetItemAtLeftHand();
	auto RightItem = GetItemAtRightHand();
	
	if (LeftItem == SlotObject)
	{
		EquipOrUnEquipHands(SlotName, SlotObject);
	}
	if (RightItem == SlotObject)
	{
		EquipOrUnEquipHands(SlotName, SlotObject);
	}
}

bool UCharacterWeaponComponent::ArmLeftHand(const FString& SlotName, UItemObject* ItemObject)
{
	if (LeftHandItem)
	{
		DisarmLeftHand();
	}
	
	LeftHandItem = SpawnWeapon(StalkerCharacter->GetMesh(), FindWeaponSlot(SlotName), ItemObject);
	if (!LeftHandItem)
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
		return false;
	}
	return true;
}

bool UCharacterWeaponComponent::ArmRightHand(const FString& SlotName, UItemObject* ItemObject)
{
	if (RightHandItem)
	{
		DisarmRightHand();
	}
	
	RightHandItem = SpawnWeapon(StalkerCharacter->GetMesh(), FindWeaponSlot(SlotName), ItemObject);
	if (!RightHandItem)
	{
		UKismetSystemLibrary::PrintString(this, FString("Hands was not equipped..."), true, false, FLinearColor::Red);
		return false;
	}
	return true;
}

void UCharacterWeaponComponent::DisarmLeftHand()
{
	if (LeftHandItem)
	{
		LeftHandItem->UnbindItem();
		LeftHandItem->Destroy();
		LeftHandItem = nullptr;
	}
}

void UCharacterWeaponComponent::DisarmRightHand()
{
	if (RightHandItem)
	{
		RightHandItem->UnbindItem();
		RightHandItem->Destroy();
		RightHandItem = nullptr;
	}
}

AItemActor* UCharacterWeaponComponent::SpawnWeapon(USceneComponent* AttachmentComponent, const FWeaponSlot* WeaponSlot,
                                                   UItemObject* ItemObject) const
{
	AItemActor* SpawnedWeapon = nullptr;
	if (AttachmentComponent->DoesSocketExist(WeaponSlot->GetAttachmentSocketName()))
	{
		if (ItemObject && ItemObject->GetActorClass())
		{
			FTransform SpawnTransform = AttachmentComponent->GetSocketTransform(WeaponSlot->GetAttachmentSocketName());
			FAttachmentTransformRules AttachmentRules{EAttachmentRule::SnapToTarget, true};

			SpawnedWeapon = GetWorld()->SpawnActorDeferred<AItemActor>(ItemObject->GetActorClass(), SpawnTransform,
																	   GetOwner(), GetOwner<APawn>(),
																	   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			SpawnedWeapon->InitializeItem(ItemObject);
			SpawnedWeapon->SetHandedMode();
			SpawnedWeapon->AttachToComponent(AttachmentComponent, AttachmentRules,
											 WeaponSlot->GetAttachmentSocketName());
			SpawnedWeapon->FinishSpawning(SpawnTransform);
		}
	}
	return SpawnedWeapon;
}

void UCharacterWeaponComponent::OnSlotEquipped(UItemObject* ItemObject, bool bModified, bool bEquipped, FString SlotName)
{
	if (!bModified || !IsValid(ItemObject) || SlotName.IsEmpty()) return;
	
	if (bEquipped)
	{	
		ArmSlot(SlotName, ItemObject);
		EquipOrUnEquipHands(SlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot equipped!"), *SlotName), true, false);
	}
	else
	{
		UnEquipHands(SlotName);
		DisarmSlot(SlotName);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot unequipped!"), *SlotName), true, false);
	}
}

UItemObject* UCharacterWeaponComponent::GetItemAtLeftHand() const
{
	UItemObject* ItemObject = nullptr;
	if (LeftHandItem)
	{
		ItemObject = LeftHandItem->GetItemObject();
	}
	return ItemObject;
}

UItemObject* UCharacterWeaponComponent::GetItemAtRightHand() const
{
	UItemObject* ItemObject = nullptr;
	if (RightHandItem)
	{
		ItemObject = RightHandItem->GetItemObject();
	}
	return ItemObject;
}

AItemActor* UCharacterWeaponComponent::GetActorAtLeftHand() const
{
	return LeftHandItem;
}

AItemActor* UCharacterWeaponComponent::GetActorAtRightHand() const
{
	return RightHandItem;
}

bool UCharacterWeaponComponent::IsLeftItemActorValid() const
{
	return IsValid(LeftHandItem.Get());
}

bool UCharacterWeaponComponent::IsRightItemActorValid() const
{
	return IsValid(RightHandItem.Get());
}

UCharacterInventoryComponent* UCharacterWeaponComponent::GetCharacterInventory() const
{
	auto CharInventory = GetOwner()->GetComponentByClass<UCharacterInventoryComponent>();
	return CharInventory;
}
