// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "DataAssets/ItemBehaviorDataAsset.h"
#include "CharacterWeaponComponent.generated.h"

class UCharacterInventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponOverlayChangedSignature, ECharacterOverlayState);

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
	TObjectPtr<UItemBehaviorDataAsset> WeaponBehaviorData;

private:
	TObjectPtr<class AStalkerCharacter> StalkerCharacter;
	TObjectPtr<UCharacterInventoryComponent> CharacterInventory;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AItemActor> LeftHandItem;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AItemActor> RightHandItem;

public:
	FOnWeaponOverlayChangedSignature OnWeaponOverlayChanged;
	
	UFUNCTION(Server, Reliable)
	void ServerToggleSlot(int8 SlotIndex);
	
protected:
	void EquipHands(const FString& SlotName, UItemObject* ItemObject, FItemBehavior* ItemBehavior);
	
	bool ArmLeftHand(const FString& SlotName, UItemObject* ItemObject);
	bool ArmRightHand(const FString& SlotName, UItemObject* ItemObject);
	void DisarmLeftHand();
	void DisarmRightHand();
	
	virtual AItemActor* SpawnWeapon(USceneComponent* AttachmentComponent, const FWeaponSlot* WeaponSlot,
									UItemObject* ItemObject) const;

	const UItemObject* GetItemAtRightHand() const;
	const UItemObject* GetItemAtLeftHand() const;
	
public:
	void OnSlotEquipped(UItemObject* ItemObject, bool bModified, FString SlotName, uint8 SlotIndex);
};
