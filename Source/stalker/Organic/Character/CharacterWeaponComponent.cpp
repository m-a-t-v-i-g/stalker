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
}

void UCharacterWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!StalkerCharacter)
	{
		return;
	}

	if (ReloadingData.bInProgress && !StalkerCharacter->CheckReloadAbility())
	{
		CancelReloadWeapon();
	}
}

void UCharacterWeaponComponent::SetupWeaponComponent()
{
	if (!IsAuthority())
	{
		return;
	}
	
	StalkerCharacter = GetOwner<AStalkerCharacter>();
	if (!StalkerCharacter)
	{
		return;
	}

	if (auto CharacterInventoryComp = StalkerCharacter->GetComponentByClass<UCharacterInventoryComponent>())
	{
		CharacterInventory = CharacterInventoryComp;
		
		for (uint8 i = 0; i < WeaponSlots.Num(); i++)
		{
			if (WeaponSlots[i].SlotName.IsEmpty())
			{
				continue;
			}
			
			if (UEquipmentSlot* SlotPtr = CharacterInventory->FindEquipmentSlot(WeaponSlots[i].SlotName))
			{
				SlotPtr->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSlotEquipped, WeaponSlots[i].SlotName);
				OnSlotEquipped(FUpdatedSlotData(SlotPtr->GetBoundObject(), SlotPtr->IsEquipped()), WeaponSlots[i].SlotName);
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
	return RightHandItem->IsA<UWeaponObject>();
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
	
	EquipOrUnEquipSlot(SlotPtr->GetSlotName(), SlotPtr->ArmedObject);
}

void UCharacterWeaponComponent::PlayBasicAction()
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

	const FItemBehavior* RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetScriptName());
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
	auto RightItem = GetItemObjectAtRightHand();
	
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
	AItemActor* RightItem = GetItemActorAtRightHand();
	
	if (!IsRightItemActorValid())
	{
		return;
	}

	if (auto RightWeapon = Cast<AWeaponActor>(RightItem))
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
	AItemActor* RightItem = GetItemActorAtRightHand();
	
	if (!IsRightItemActorValid())
	{
		return;
	}

	if (auto RightWeapon = Cast<AWeaponActor>(RightItem))
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
	
	int AmmoCount = Ammo->GetItemInstance()->Amount;
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

		GetCharacterInventory()->ServerSubtractOrRemoveItem(ReloadingData.AmmoObject->GetItemId(), ReloadingData.AmmoCount);
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
		int AmmoCount = Ammo->GetItemInstance()->Amount;
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
	if (!IncomingItem || SlotName.IsEmpty())
	{
		return;
	}

	if (!WeaponBehaviorConfig)
	{
		return;
	}
	
	const FItemBehavior* WeaponConfig = WeaponBehaviorConfig->ItemsMap.Find(IncomingItem->GetScriptName());
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
		RightItemBeh = WeaponBehaviorConfig->ItemsMap.Find(RightItem->GetScriptName());
	}
	
	switch (WeaponConfig->OccupiedHand)
	{
	case EOccupiedHand::Left:
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
				if (!LeftItem)
				{
					TargetOverlay = ECharacterOverlayState::OnlyRightHand;
				}
				else
				{
					TargetOverlay = ECharacterOverlayState::LeftAndRightHand;
				}
				ArmRightHand(SlotName, IncomingItem);
			}
			else
			{
				if (!LeftItem)
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
				TargetOverlay = ECharacterOverlayState::BothHands;
				
				if (LeftItem)
				{
					DisarmLeftHand();
				}
				ArmRightHand(SlotName, IncomingItem);
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

	EquipOrUnEquipSlot(SlotName, SlotPtr->ArmedObject);
}

bool UCharacterWeaponComponent::ArmLeftHand(const FString& SlotName, UItemObject* ItemObject)
{
	if (LeftHandItem)
	{
		DisarmLeftHand();
	}
	
	LeftHandItem = SpawnWeapon(StalkerCharacter->GetMesh(), ItemObject, FCharacterSocketName::NAME_LeftHand);
	
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
	
	RightHandItem = SpawnWeapon(StalkerCharacter->GetMesh(), ItemObject, FCharacterSocketName::NAME_RightHand);
	
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
		LeftHandItem->UnbindItemObject();
		LeftHandItem->Destroy();
		LeftHandItem = nullptr;
	}
}

void UCharacterWeaponComponent::DisarmRightHand()
{
	if (RightHandItem)
	{
		RightHandItem->UnbindItemObject();
		RightHandItem->Destroy();
		RightHandItem = nullptr;
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
			                                                                StalkerCharacter, StalkerCharacter,
			                                                                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (SpawnedWeapon)
			{
				SpawnedWeapon->BindItemObject(ItemObject);
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
		EquipOrUnEquipSlot(SlotName, SlotData.SlotItem);
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
	if (LeftHandItem)
	{
		return  LeftHandItem->GetItemObject();
	}
	return nullptr;
}

UItemObject* UCharacterWeaponComponent::GetItemObjectAtRightHand() const
{
	if (RightHandItem)
	{
		return RightHandItem->GetItemObject();
	}
	return nullptr;
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
