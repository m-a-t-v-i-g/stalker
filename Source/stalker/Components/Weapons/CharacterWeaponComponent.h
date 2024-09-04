// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "DataAssets/ItemBehaviorDataAsset.h"
#include "CharacterWeaponComponent.generated.h"

class UCharacterInventoryComponent;

DECLARE_MULTICAST_DELEGATE(FOnAimingDelegate);
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
	virtual bool IsArmed() const override;
	
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

	UPROPERTY(Replicated)
	bool bAiming = false;
	
public:
	FOnAimingDelegate OnAimingStart;
	FOnAimingDelegate OnAimingStop;
	FOnWeaponOverlayChangedSignature OnWeaponOverlayChanged;
	
	UFUNCTION(Server, Reliable)
	void ServerToggleSlot(int8 SlotIndex);
	
	void PlayBasicAction();
	void StopBasicAction();
	
	void PlayAlternativeAction();
	void StopAlternativeAction();

	void StartFire();
	
	UFUNCTION(Server, Unreliable)
	void ServerStartFire();
	
	void StopFire();
	
	UFUNCTION(Server, Unreliable)
	void ServerStopFire();
	
	void StartAiming();
	
	UFUNCTION(Server, Unreliable)
	void ServerStartAiming();
	
	void StopAiming();
	
	UFUNCTION(Server, Unreliable)
	void ServerStopAiming();
	
protected:
	void EquipOrUnEquipHands(const FString& SlotName, UItemObject* ItemObject);
	void UnEquipHands(const FString& SlotName);
	
	bool ArmLeftHand(const FString& SlotName, UItemObject* ItemObject);
	bool ArmRightHand(const FString& SlotName, UItemObject* ItemObject);
	void DisarmLeftHand();
	void DisarmRightHand();
	
	virtual AItemActor* SpawnWeapon(USceneComponent* AttachmentComponent, const FWeaponSlot* WeaponSlot,
									UItemObject* ItemObject) const;

public:
	void OnSlotEquipped(UItemObject* ItemObject, bool bModified, bool bEquipped, FString SlotName);
	
	FORCEINLINE const UItemObject* GetItemAtLeftHand() const;
	FORCEINLINE const UItemObject* GetItemAtRightHand() const;

	FORCEINLINE AItemActor* GetActorAtLeftHand() const;
	FORCEINLINE AItemActor* GetActorAtRightHand() const;
	FORCEINLINE bool IsLeftItemActorValid() const;
	FORCEINLINE bool IsRightItemActorValid() const;
};
