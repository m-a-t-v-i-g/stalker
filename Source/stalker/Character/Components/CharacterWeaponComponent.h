// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "CharacterOutfitComponent.h"
#include "Ammo/AmmoObject.h"
#include "Data/ItemBehaviorConfig.h"
#include "CharacterWeaponComponent.generated.h"

struct FUpdatedSlotData;
class UCharacterInventoryComponent;
class UCharacterStateComponent;
class UWeaponObject;
class AStalkerCharacter;

DECLARE_MULTICAST_DELEGATE(FCharacterFireDelegate);
DECLARE_MULTICAST_DELEGATE(FCharacterAimingDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterStartReloadSignature, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterStopReloadSignature, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWeaponOverlayChangedSignature, ECharacterOverlayState);

USTRUCT()
struct FEquippedWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<UItemObject> ItemObject;

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<AItemActor> ItemActor;

	UPROPERTY(VisibleInstanceOnly)
	FWeaponBehavior WeaponBehavior;
	
	FEquippedWeaponData() {}
	
	FEquippedWeaponData(UItemObject* ItemObj, AItemActor* ItemAct, const FWeaponBehavior& ItemBeh) : ItemObject(ItemObj),
		ItemActor(ItemAct), WeaponBehavior(ItemBeh)
	{
	}

	void Clear()
	{
		ItemObject = nullptr;
		ItemActor = nullptr;
		WeaponBehavior.Clear();
	}

	bool IsValid() const
	{
		return ItemObject != nullptr && ItemActor != nullptr;
	}
};

USTRUCT()
struct FReloadingData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UWeaponObject* WeaponObject = nullptr;

	UPROPERTY()
	UAmmoObject* AmmoObject = nullptr;

	uint16 AmmoCount = 0;

	float ReloadTime = 0.0f;

	bool bInProgress = false;
	
	FReloadingData() {}
	
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

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterWeaponComponent : public UCharacterOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterWeaponComponent();

	FCharacterFireDelegate OnFireStart;
	FCharacterFireDelegate OnFireStop;
	FCharacterAimingDelegate OnAimingStart;
	FCharacterAimingDelegate OnAimingStop;
	FOnCharacterStartReloadSignature OnReloadStart;
	FOnCharacterStopReloadSignature OnReloadStop;
	FOnWeaponOverlayChangedSignature OnOverlayChanged;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnCharacterDead() override;

	void TryToggleSlot(int8 SlotIndex);
	
	UFUNCTION(Server, Reliable)
	void ServerToggleSlot(int8 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PlayBasicAction();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopBasicAction();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PlayAlternativeAction();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopAlternativeAction();

	void StartFire();
	
	UFUNCTION(Server, Reliable)
	void ServerStartFire();
	
	void StopFire();
	
	UFUNCTION(Server, Reliable)
	void ServerStopFire();
	
	void StartAlternative();
	
	UFUNCTION(Server, Reliable)
	void ServerStartAlternative();
	
	void StopAlternative();
	
	UFUNCTION(Server, Reliable)
	void ServerStopAlternative();
	
	void StartAiming();
	
	UFUNCTION(Server, Unreliable)
	void ServerStartAiming();
	
	void StopAiming();
	
	UFUNCTION(Server, Unreliable)
	void ServerStopAiming();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
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
	UAmmoObject* GetAmmoForReload(const UAmmoDefinition* DesiredAmmo) const;
	
	const FWeaponBehavior* GetWeaponBehavior(const FName& ItemScriptName) const;
	
	FVector GetFireLocation() const;
	
	FORCEINLINE UItemObject* GetItemObjectAtLeftHand() const;
	
	template<class T>
	T* GetItemObjectAtLeftHand() const
	{
		return Cast<T>(GetItemObjectAtLeftHand());
	}
	
	FORCEINLINE UItemObject* GetItemObjectAtRightHand() const;

	template<class T>
	T* GetItemObjectAtRightHand() const
	{
		return Cast<T>(GetItemObjectAtRightHand());
	}

	FORCEINLINE AItemActor* GetItemActorAtLeftHand() const;
	
	template<class T>
	T* GetItemActorAtLeftHand() const
	{
		return Cast<T>(GetItemActorAtLeftHand());
	}
	
	FORCEINLINE AItemActor* GetItemActorAtRightHand() const;
	
	template<class T>
	T* GetItemActorAtRightHand() const
	{
		return Cast<T>(GetItemActorAtRightHand());
	}

	FORCEINLINE bool IsLeftItemObjectValid() const;
	FORCEINLINE bool IsRightItemObjectValid() const;
	
	FORCEINLINE bool IsLeftItemActorValid() const;
	FORCEINLINE bool IsRightItemActorValid() const;
	
protected:
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem) override;
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem) override;
	
	void EquipOrUnequipSlot(const FString& SlotName, UItemObject* IncomingItem);

	void ArmHand(FEquippedWeaponData& HandedItemData, AItemActor*& ReplicatedItemActor, const FName& SocketName, UItemObject* ItemObject);
	void DisarmHand(FEquippedWeaponData& HandedItemData, AItemActor*& ReplicatedItemActor);
	
	virtual AItemActor* SpawnWeapon(USceneComponent* AttachmentComponent, UItemObject* ItemObject, FName SocketName) const;
	
private:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FEquippedWeaponData LeftHandItemData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FEquippedWeaponData RightHandItemData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	AItemActor* LeftHandItemActor = nullptr;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	AItemActor* RightHandItemActor = nullptr;

	FReloadingData ReloadingData;

	FTimerHandle ReloadTimerHandle;
};
