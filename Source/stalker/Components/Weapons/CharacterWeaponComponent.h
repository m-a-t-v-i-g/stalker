// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "CharacterWeaponComponent.generated.h"

class UCharacterInventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, UItemObject*);

UCLASS(meta=(BlueprintSpawnableComponent))
class STALKER_API UCharacterWeaponComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	UCharacterWeaponComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreInitializeWeapon() override;
	virtual void PostInitializeWeapon() override;

	virtual bool CanAttack() const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString KnifeSlotName = "Knife";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString MainSlotName = "Main";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString SecondarySlotName = "Secondary";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString GrenadeSlotName = "Grenade";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString BinocularsSlotName = "Binoculars";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString BoltSlotName = "Bolt";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString DetectorSlotName = "Detector";

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<class UCharacterItemsDataAsset> ItemsBehaviorData;

private:
	TObjectPtr<class AStalkerCharacter> StalkerCharacter;
	
	TObjectPtr<UCharacterInventoryComponent> CharacterInventory;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AItemActor> LeftHandItem;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AItemActor> RightHandItem;

public:
	FOnWeaponChanged OnWeaponChanged;
	
protected:
	bool ArmLeftHand(const FString& SlotName, UItemObject* ItemObject);
	bool ArmRightHand(const FString& SlotName, UItemObject* ItemObject);
	void DisarmLeftHand();
	void DisarmRightHand();
	
	virtual AItemActor* SpawnWeapon(USceneComponent* AttachmentComponent, const FWeaponSlot* WeaponSlot,
									UItemObject* ItemObject) const;
	
public:
	void OnKnifeSlotEquipped(UItemObject* ItemObject, bool bModified);
	void OnKnifeSlotActivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject);
	
	void OnMainSlotEquipped(UItemObject* ItemObject, bool bModified);
	void OnMainSlotActivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject);
	void OnMainSlotDeactivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject);
	
	void OnSecondarySlotEquipped(UItemObject* ItemObject, bool bModified);
	void OnSecondarySlotActivated(const FString& SlotName, int8 SlotIndex, UItemObject* ItemObject);
	
	void OnGrenadeSlotEquipped(UItemObject* ItemObject, bool bModified);
	void OnBinocularsSlotEquipped(UItemObject* ItemObject, bool bModified);
	void OnBoltSlotEquipped(UItemObject* ItemObject, bool bModified);
	void OnDetectorSlotEquipped(UItemObject* ItemObject, bool bModified);
};
