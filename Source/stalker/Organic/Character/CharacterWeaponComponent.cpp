// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"
#include "ItemActor.h"
#include "ItemObject.h"
#include "Ammo/AmmoObject.h"
#include "Character/CharacterInventoryComponent.h"
#include "Character/StalkerCharacter.h"
#include "Components/EquipmentSlot.h"
#include "Data/ItemBehaviorDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponActor.h"
#include "Weapons/WeaponObject.h"

UCharacterWeaponComponent::UCharacterWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterWeaponComponent, LeftHandItem);
	DOREPLIFETIME(UCharacterWeaponComponent, RightHandItem);
	DOREPLIFETIME(UCharacterWeaponComponent, bAiming);
}

void UCharacterWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!StalkerCharacter.Get()) return;

	if (ReloadingData.bInProgress && !StalkerCharacter->CheckReloadAbility())
	{
		CancelReloadWeapon();
	}
}

void UCharacterWeaponComponent::PreInitializeWeapon()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	Super::PreInitializeWeapon();
	
	StalkerCharacter = GetOwner<AStalkerCharacter>();
	if (!StalkerCharacter.Get())
	{
		return;
	}

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
	
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
}

bool UCharacterWeaponComponent::CanAttack() const
{
	return Super::CanAttack();
}

bool UCharacterWeaponComponent::IsArmed() const
{
	return RightHandItem->IsA<UWeaponObject>();
}

void UCharacterWeaponComponent::ServerToggleSlot_Implementation(int8 SlotIndex)
{
	auto* SlotPtr = &WeaponSlots[SlotIndex];
	if (!SlotPtr)
	{
		return;
	}

	if (!SlotPtr->IsArmed())
	{
		return;
	}
	
	auto SlotObject = SlotPtr->ArmedItemObject;
	EquipOrUnEquipSlot(SlotPtr->GetSlotName(), SlotObject);
}

void UCharacterWeaponComponent::PlayBasicAction()
{
	auto RightItem = GetItemObjectAtRightHand();
	
	if (!RightItem || !IsRightItemActorValid())
	{
		return;
	}

	FItemBehavior* RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetItemRowName());
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
	auto RightItem = GetItemObjectAtRightHand();
	
	if (!RightItem || !IsRightItemActorValid())
	{
		return;
	}

	FItemBehavior* RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetItemRowName());
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
	auto RightItem = GetItemObjectAtRightHand();
	
	if (!RightItem || !IsRightItemActorValid())
	{
		return;
	}

	FItemBehavior* RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetItemRowName());
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
	auto RightItem = GetItemObjectAtRightHand();
	
	if (!RightItem || !IsRightItemActorValid())
	{
		return;
	}

	FItemBehavior* RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetItemRowName());
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
	auto RightItem = GetItemActorAtRightHand();
	
	if (!IsRightItemActorValid())
	{
		return;
	}

	if (auto RightWeapon = Cast<AWeaponActor>(RightItem))
	{
		if (!bAiming)
		{
			OnAimingStart.Broadcast();
		}
		
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
	auto RightItem = GetItemActorAtRightHand();
	
	if (!IsRightItemActorValid())
	{
		return;
	}

	if (auto RightWeapon = Cast<AWeaponActor>(RightItem))
	{
		if (!bAiming)
		{
			OnAimingStop.Broadcast();
		}
		
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
	auto RightItem = GetItemObjectAtRightHand<UWeaponObject>();
	if (!RightItem || RightItem->IsMagFull() || !HasAmmoForReload()) return;
	
	if (IsAutonomousProxy())
	{
		ReloadingData.ReloadTime = RightItem->GetWeaponParams().ReloadTime;
		SetReloadTimer();
	}
	
	ServerTryReloadWeapon();
}

void UCharacterWeaponComponent::ServerTryReloadWeapon_Implementation()
{
	auto RightWeapon = GetItemObjectAtRightHand<UWeaponObject>();
	if (!RightWeapon || RightWeapon->IsMagFull() || !HasAmmoForReload()) return;

	UAmmoObject* Ammo = GetAmmoForReload();
	if (!Ammo) return;
	
	int AmmoCount = Ammo->GetItemParams().Amount;
	if (AmmoCount > 0)
	{
		int RequiredCount = RightWeapon->CalculateRequiredAmmoCount();
		int AvailableCount = FMath::Min(RequiredCount, AmmoCount);
		
		ReloadingData = FReloadingData(RightWeapon, Ammo, AvailableCount, RightWeapon->GetWeaponParams().ReloadTime, true);
		
		SetReloadTimer();
		MulticastReloadWeapon(ReloadingData.ReloadTime);
	}
}

void UCharacterWeaponComponent::MulticastReloadWeapon_Implementation(float ReloadTime)
{
	if (IsSimulatedProxy())
	{
		ReloadingData.ReloadTime = ReloadTime;
		SetReloadTimer();
	}
}

void UCharacterWeaponComponent::CompleteReloadWeapon()
{
	if (GetOwner()->HasAuthority())
	{
		if (!ReloadingData.IsValid())
		{
			return;
		}

		GetCharacterInventory()->SubtractOrRemoveItem(ReloadingData.AmmoObject, ReloadingData.AmmoCount);
		ReloadingData.WeaponObject->IncreaseAmmo(ReloadingData.AmmoCount);
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
	
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, ReloadDelegate, ReloadingData.ReloadTime, false);
	OnReloadStart.Broadcast(ReloadingData.ReloadTime);

	UKismetSystemLibrary::PrintString(this, FString("Reload started..."), true, false);
}

void UCharacterWeaponComponent::ClearReloadingData(bool bWasSuccessful)
{
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	OnReloadStop.Broadcast(bWasSuccessful);
	ReloadingData.Clear();
}

bool UCharacterWeaponComponent::HasAmmoForReload() const
{
	if (UAmmoObject* Ammo = GetAmmoForReload())
	{
		int AmmoCount = Ammo->GetItemParams().Amount;
		return AmmoCount > 0;
	}
	return false;
}

UAmmoObject* UCharacterWeaponComponent::GetAmmoForReload() const
{
	UAmmoObject* ResultAmmo = nullptr;
	if (auto RightItem = GetItemObjectAtRightHand<UWeaponObject>())
	{
		auto Params = RightItem->GetWeaponParams();
		for (auto AmmoClass : Params.AmmoClasses)
		{
			// TODO: ResultAmmo = Cast<UAmmoObject>(GetCharacterInventory()->FindItemByClass(AmmoClass.Get()));
			if (ResultAmmo)
			{
				break;
			}
		}
	}
	return ResultAmmo;
}

void UCharacterWeaponComponent::EquipOrUnEquipSlot(const FString& SlotName, UItemObject* IncomingItem)
{
	if (!IncomingItem)
	{
		return;
	}
	
	const auto WeaponConfig = WeaponBehaviorConfig->ItemsMap.Find(IncomingItem->GetItemRowName());
	if (!WeaponConfig)
	{
		UKismetSystemLibrary::PrintString(
			this, FString::Printf(TEXT("Behavior for item '%s' not found!"), *IncomingItem->GetItemRowName().ToString()),
			true, false, FLinearColor::Red);
		return;
	}
	
	ECharacterOverlayState TargetOverlay = ECharacterOverlayState::Default;
	
	auto LeftItem = GetItemObjectAtLeftHand();
	auto RightItem = GetItemObjectAtRightHand();
	
	FItemBehavior* RightItemBeh = nullptr;
	if (RightItem)
	{
		RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetItemRowName());
	}
	
	switch (WeaponConfig->OccupiedHand)
	{
	case ECharacterSlotHand::Left:
		{
			if (LeftItem != IncomingItem)
			{
				if (RightItem)
				{
					if (!RightItemBeh)
					{
						return;
					}
					
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
				
				ArmLeftHand(SlotName, IncomingItem);
			}
			else
			{
				if (RightItem)
				{
					if (!RightItemBeh)
					{
						return;
					}
					
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
			TargetOverlay = LeftItem ? WeaponConfig->SecondaryOverlay : WeaponConfig->PrimaryOverlay;
			
			if (RightItem != IncomingItem)
			{
				ArmRightHand(SlotName, IncomingItem);
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
			TargetOverlay = WeaponConfig->PrimaryOverlay;
			
			if (RightItem != IncomingItem)
			{
				if (LeftItem)
				{
					DisarmLeftHand();
				}
				
				ArmRightHand(SlotName, IncomingItem);
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

void UCharacterWeaponComponent::UnEquipSlot(const FString& SlotName)
{
	auto SlotPtr = FindWeaponSlot(SlotName);
	if (!SlotPtr)
	{
		return;
	}

	if (!SlotPtr->IsArmed())
	{
		return;
	}

	auto SlotObject = SlotPtr->ArmedItemObject;
	auto LeftItem = GetItemObjectAtLeftHand();
	auto RightItem = GetItemObjectAtRightHand();
	
	if (LeftItem == SlotObject)
	{
		EquipOrUnEquipSlot(SlotName, SlotObject);
	}
	
	if (RightItem == SlotObject)
	{
		EquipOrUnEquipSlot(SlotName, SlotObject);
	}
}

bool UCharacterWeaponComponent::ArmLeftHand(const FString& SlotName, UItemObject* ItemObject)
{
	if (LeftHandItem)
	{
		DisarmLeftHand();
	}
	
	LeftHandItem = SpawnWeapon(StalkerCharacter->GetMesh(), FindWeaponSlot(SlotName), ItemObject);
	
	if (!LeftHandItem.Get())
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
	
	if (!RightHandItem.Get())
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
																	   StalkerCharacter, StalkerCharacter,
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
		EquipOrUnEquipSlot(SlotName, ItemObject);
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
	UItemObject* ItemObject = nullptr;
	if (LeftHandItem)
	{
		ItemObject = LeftHandItem->GetItemObject();
	}
	return ItemObject;
}

UItemObject* UCharacterWeaponComponent::GetItemObjectAtRightHand() const
{
	UItemObject* ItemObject = nullptr;
	if (RightHandItem)
	{
		ItemObject = RightHandItem->GetItemObject();
	}
	return ItemObject;
}

AItemActor* UCharacterWeaponComponent::GetItemActorAtLeftHand() const
{
	return LeftHandItem;
}

AItemActor* UCharacterWeaponComponent::GetItemActorAtRightHand() const
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
