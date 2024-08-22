// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"

#include "ItemActor.h"
#include "Characters/StalkerCharacter.h"
#include "DataAssets/ItemBehaviorDataAsset.h"
#include "Inventory/CharacterInventoryComponent.h"
#include "Inventory/EquipmentSlot.h"
#include "Items/ItemObject.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UCharacterWeaponComponent::UCharacterWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterWeaponComponent, LeftHandItem);
	DOREPLIFETIME(UCharacterWeaponComponent, RightHandItem);
}

void UCharacterWeaponComponent::PreInitializeWeapon()
{
	StalkerCharacter = GetOwner<AStalkerCharacter>();
	if (!StalkerCharacter.Get()) return;
	
	if (auto CharacterInventoryComp = StalkerCharacter->GetComponentByClass<UCharacterInventoryComponent>())
	{
		CharacterInventory = CharacterInventoryComp;

		for (uint8 i = 0; i < WeaponSlotNames.Num(); i++)
		{
			if (WeaponSlotNames[i].IsEmpty()) continue;
			if (auto SlotPtr = CharacterInventory->FindEquipmentSlot(WeaponSlotNames[i]))
			{
				SlotPtr->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSlotEquipped, WeaponSlotNames[i], i);
			}
		}
	}
	Super::PreInitializeWeapon();
}

void UCharacterWeaponComponent::PostInitializeWeapon()
{
	Super::PostInitializeWeapon();
}

bool UCharacterWeaponComponent::CanAttack() const
{
	return Super::CanAttack();
}

void UCharacterWeaponComponent::ServerToggleSlot_Implementation(int8 SlotIndex)
{
	auto* SlotPtr = &WeaponSlots[SlotIndex];
	if (!SlotPtr) return;

	auto SlotObject = SlotPtr->ArmedItemObject;
	if (!SlotObject || !SlotPtr->IsArmed()) return;

	const auto ItemBehavior = WeaponBehaviorData->ItemsMap.Find(SlotObject->GetItemRowName());
	if (!ItemBehavior)
	{
		UKismetSystemLibrary::PrintString(this, FString("Behavior for item not found!"), true, false,
		                                  FLinearColor::Red);
		return;
	}

	EquipHands(SlotPtr->GetSlotName(), SlotObject, ItemBehavior->OccupiedHand);

	if (auto SlotActivityPtr = ActiveSlots.Find(SlotIndex))
	{
		if (*SlotActivityPtr)
		{
			ServerDeactivateSlot_Implementation(SlotIndex);
		}
		else
		{
			ServerActivateSlot_Implementation(SlotIndex);
		}
	}
}

void UCharacterWeaponComponent::EquipHands(const FString& SlotName, UItemObject* ItemObject, ECharacterSlotHand TargetHand)
{
	switch (TargetHand)
	{
	case ECharacterSlotHand::Left:
		{
			const auto RightItemBeh = WeaponBehaviorData->ItemsMap.Find(GetItemAtRightHand()->GetItemRowName());
			if (!RightItemBeh) return;

			switch (RightItemBeh->OccupiedHand)
			{
			case ECharacterSlotHand::Both:
				DisarmRightHand();
				break;
			default: break;
			}
			ArmLeftHand(SlotName, ItemObject);
			break;
		}
	case ECharacterSlotHand::Right:
		{
			ArmRightHand(SlotName, ItemObject);
			break;
		}
	case ECharacterSlotHand::Both:
		{
			DisarmLeftHand();
			ArmRightHand(SlotName, ItemObject);
			break;
		}
	default: break;
	}
}

void UCharacterWeaponComponent::OnActivateSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (!GetOwner()->HasAuthority() || !IsValid(ItemObject)) return;

	const auto ItemBehavior = WeaponBehaviorData->ItemsMap.Find(ItemObject->GetItemRowName());
	if (!ItemBehavior)
	{
		UKismetSystemLibrary::PrintString(this, FString("Behavior for item not found!"), true, false,
										  FLinearColor::Red);
		return;
	}
}

void UCharacterWeaponComponent::OnDeactivateSlot(const FString& SlotName, UItemObject* ItemObject)
{
	if (!GetOwner()->HasAuthority() || !IsValid(ItemObject)) return;

	const auto ItemBehavior = WeaponBehaviorData->ItemsMap.Find(ItemObject->GetItemRowName());
	if (!ItemBehavior)
	{
		UKismetSystemLibrary::PrintString(this, FString("Behavior for item not found!"), true, false,
										  FLinearColor::Red);
		return;
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
		UKismetSystemLibrary::PrintString(this, FString("Hand was not equipped..."), true, false, FLinearColor::Red);
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
		UKismetSystemLibrary::PrintString(this, FString("Hand was not equipped..."), true, false, FLinearColor::Red);
		return false;
	}
	return true;
}

void UCharacterWeaponComponent::DisarmLeftHand()
{
	if (LeftHandItem)
	{
		LeftHandItem->Destroy();
		LeftHandItem = nullptr;
	}
}

void UCharacterWeaponComponent::DisarmRightHand()
{
	if (RightHandItem)
	{
		RightHandItem->Destroy();
		RightHandItem = nullptr;
	}
}

AItemActor* UCharacterWeaponComponent::SpawnWeapon(USceneComponent* AttachmentComponent, const FWeaponSlot* WeaponSlot,
                                                   UItemObject* ItemObject) const
{
	check(AttachmentComponent && WeaponSlot);
	
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
			SpawnedWeapon->InitItem(ItemObject);
			SpawnedWeapon->SetHandedMode();
			SpawnedWeapon->AttachToComponent(AttachmentComponent, AttachmentRules,
											 WeaponSlot->GetAttachmentSocketName());
			SpawnedWeapon->FinishSpawning(SpawnTransform);
		}
	}
	return SpawnedWeapon;
}

const UItemObject* UCharacterWeaponComponent::GetItemAtRightHand() const
{
	const UItemObject* ItemObject = nullptr;
	if (RightHandItem)
	{
		ItemObject = RightHandItem->GetItemObject();
	}
	return ItemObject;
}

const UItemObject* UCharacterWeaponComponent::GetItemAtLeftHand() const
{
	const UItemObject* ItemObject = nullptr;
	if (LeftHandItem)
	{
		ItemObject = LeftHandItem->GetItemObject();
	}
	return ItemObject;
}

void UCharacterWeaponComponent::OnSlotEquipped(UItemObject* ItemObject, bool bModified, FString SlotName, uint8 SlotIndex)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{	
		ArmSlot(SlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot equipped!"), *SlotName), true, false);
	}
	else
	{
		DisarmSlot(SlotName);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s slot unequipped!"), *SlotName), true, false);
	}
	ServerToggleSlot_Implementation(SlotIndex);
}
