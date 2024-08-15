// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterWeaponComponent.h"

#include "ItemActor.h"
#include "Characters/StalkerCharacter.h"
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
		
		if (auto KnifeSlot = CharacterInventory->FindEquipmentSlot(KnifeSlotName))
		{
			KnifeSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnKnifeSlotEquipped);
		}
		
		if (auto MainSlot = CharacterInventory->FindEquipmentSlot(MainSlotName))
		{
			MainSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnMainSlotEquipped);
			OnSlotActivated.AddUObject(this, &UCharacterWeaponComponent::OnMainSlotActivated);
			OnSlotDeactivated.AddUObject(this, &UCharacterWeaponComponent::OnMainSlotDeactivated);
		}
		
		if (auto SecondarySlot = CharacterInventory->FindEquipmentSlot(SecondarySlotName))
		{
			SecondarySlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnSecondarySlotEquipped);
		}
		
		if (auto GrenadeSlot = CharacterInventory->FindEquipmentSlot(GrenadeSlotName))
		{
			GrenadeSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnGrenadeSlotEquipped);
		}
		
		if (auto BinocularsSlot = CharacterInventory->FindEquipmentSlot(BinocularsSlotName))
		{
			BinocularsSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnBinocularsSlotEquipped);
		}
		
		if (auto DetectorSlot = CharacterInventory->FindEquipmentSlot(DetectorSlotName))
		{
			DetectorSlot->OnSlotChanged.AddUObject(this, &UCharacterWeaponComponent::OnDetectorSlotEquipped);
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

void UCharacterWeaponComponent::OnKnifeSlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{
		ArmSlot(KnifeSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Knife equipped!"), true, false);
	}
	else
	{
		DisarmSlot(KnifeSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Knife unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnKnifeSlotActivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject)
{
	if (SlotName == KnifeSlotName)
	{
		ArmRightHand(SlotName, ItemObject);
	}
}

void UCharacterWeaponComponent::OnMainSlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject))
	{
		ArmSlot(MainSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Rifle equipped!"), true, false);
	}
	else
	{
		DisarmSlot(MainSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Rifle unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnMainSlotActivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject)
{
	if (SlotName == MainSlotName)
	{
		ArmRightHand(SlotName, ItemObject);
	}
}

void UCharacterWeaponComponent::OnMainSlotDeactivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject)
{
	if (SlotName == MainSlotName)
	{
		DisarmRightHand();
	}
}

void UCharacterWeaponComponent::OnSecondarySlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(SecondarySlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Pistol equipped!"), true, false);
	}
	else
	{
		DisarmSlot(SecondarySlotName);
		UKismetSystemLibrary::PrintString(this, FString("Pistol unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnSecondarySlotActivated(const FString& SlotName, int8 SlotIndex,
                                                         UItemObject* ItemObject)
{
	if (SlotName == SecondarySlotName)
	{
		ArmRightHand(SlotName, ItemObject);
	}
}

void UCharacterWeaponComponent::OnGrenadeSlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(GrenadeSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Grenade equipped!"), true, false);
	}
	else
	{
		DisarmSlot(GrenadeSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Grenade unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnBinocularsSlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(BinocularsSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Binocular equipped!"), true, false);
	}
	else
	{
		DisarmSlot(BinocularsSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Binocular unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnBoltSlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(BoltSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Bolt equipped!"), true, false);
	}
	else
	{
		DisarmSlot(BoltSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Bolt unequipped!"), true, false);
	}
}

void UCharacterWeaponComponent::OnDetectorSlotEquipped(UItemObject* ItemObject, bool bModified)
{
	if (!bModified) return;
	
	if (IsValid(ItemObject) && bModified)
	{
		ArmSlot(DetectorSlotName, ItemObject);
		UKismetSystemLibrary::PrintString(this, FString("Detector equipped!"), true, false);
	}
	else
	{
		DisarmSlot(DetectorSlotName);
		UKismetSystemLibrary::PrintString(this, FString("Detector unequipped!"), true, false);
	}
}
