// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "Ammo/AmmoObject.h"
#include "DataAssets/ItemBehaviorDataAsset.h"
#include "CharacterWeaponComponent.generated.h"

class UWeaponObject;
class UCharacterInventoryComponent;

DECLARE_MULTICAST_DELEGATE(FCharacterAimingDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterStartReloadSignature, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterStopReloadSignature, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponOverlayChangedSignature, ECharacterOverlayState);

USTRUCT()
struct FReloadingData
{
	GENERATED_USTRUCT_BODY()
	
	UWeaponObject* WeaponObject = nullptr;

	UAmmoObject* AmmoObject = nullptr;

	uint16 AmmoCount = 0;

	float ReloadTime = 0.0f;

	bool bInProgress = false;
	
	FReloadingData() = default;
	
	FReloadingData(UWeaponObject* WeaponObj, UAmmoObject* AmmoObj, uint16 AmmoAmount, float TimeToReload,
	               bool bProcessReload) : WeaponObject(WeaponObj), AmmoObject(AmmoObj), AmmoCount(AmmoAmount),
	                                      ReloadTime(TimeToReload), bInProgress(bProcessReload)
	{
	}

	void Clear()
	{
		WeaponObject = nullptr;
		AmmoObject = nullptr;
		AmmoCount = 0;
		ReloadTime = 0.0f;
		bInProgress = false;
	}

	bool IsValid() const
	{
		return WeaponObject != nullptr && AmmoObject != nullptr && AmmoCount > 0;
	}
};

UCLASS(meta=(BlueprintSpawnableComponent))
class STALKER_API UCharacterWeaponComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	UCharacterWeaponComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	FReloadingData ReloadingData;

	FTimerHandle ReloadTimer;
	
public:
	FCharacterAimingDelegate OnAimingStart;
	FCharacterAimingDelegate OnAimingStop;
	FOnCharacterStartReloadSignature OnReloadStart;
	FOnCharacterStopReloadSignature OnReloadStop;
	FOnWeaponOverlayChangedSignature OnWeaponOverlayChanged;
	
	UFUNCTION(Server, Reliable)
	void ServerToggleSlot(int8 SlotIndex);
	
	void PlayBasicAction();
	void StopBasicAction();
	
	void PlayAlternativeAction();
	void StopAlternativeAction();

	void StartFire();
	
	UFUNCTION(Server, Reliable)
	void ServerStartFire();
	
	void StopFire();
	
	UFUNCTION(Server, Reliable)
	void ServerStopFire();
	
	void StartAiming();
	
	UFUNCTION(Server, Unreliable)
	void ServerStartAiming();
	
	void StopAiming();
	
	UFUNCTION(Server, Unreliable)
	void ServerStopAiming();

	void TryReloadWeapon();
	
	UFUNCTION(Server, Reliable)
	void ServerTryReloadWeapon();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastReloadWeapon(float ReloadTime);
	
	void CompleteReloadWeapon();
	void CancelReloadWeapon();

	void SetReloadTimer();
	void ClearReloadingData(bool bWasSuccessful);

	bool HasAmmoForReload() const;
	UAmmoObject* GetAmmoForReload() const;
	
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
	
	FORCEINLINE UItemObject* GetItemAtLeftHand() const;
	FORCEINLINE UItemObject* GetItemAtRightHand() const;

	template<class T>
	T* GetItemAtRightHand() const
	{
		return Cast<T>(GetItemAtRightHand());
	}

	FORCEINLINE AItemActor* GetActorAtLeftHand() const;
	FORCEINLINE AItemActor* GetActorAtRightHand() const;
	
	template<class T>
	T* GetActorAtRightHand() const
	{
		return Cast<T>(GetActorAtRightHand());
	}
	
	FORCEINLINE bool IsLeftItemActorValid() const;
	FORCEINLINE bool IsRightItemActorValid() const;

	UCharacterInventoryComponent* GetCharacterInventory() const;
};
