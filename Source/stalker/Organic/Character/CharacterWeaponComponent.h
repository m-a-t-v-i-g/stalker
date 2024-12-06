// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "Ammo/AmmoObject.h"
#include "Components/WeaponComponent.h"
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
struct FHandedItemData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<UItemObject> ItemObject;

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<AItemActor> ItemActor;

	UPROPERTY(VisibleInstanceOnly)
	FItemBehavior ItemBehavior;
	
	FHandedItemData() {}
	
	FHandedItemData(UItemObject* ItemObj, AItemActor* ItemAct, const FItemBehavior& ItemBeh) : ItemObject(ItemObj),
		ItemActor(ItemAct), ItemBehavior(ItemBeh)
	{
	}

	void Clear()
	{
		ItemObject = nullptr;
		ItemActor = nullptr;
		ItemBehavior.Clear();
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
	
	UWeaponObject* WeaponObject = nullptr;

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
class STALKER_API UCharacterWeaponComponent : public UWeaponComponent
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

	virtual void SetupWeaponComponent(AStalkerCharacter* InCharacter);
	virtual void InitCharacterInfo(AController* InController);

	virtual bool CanAttack() const override;
	virtual bool IsArmed() const override;

	FVector GetFireLocation() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<const UItemBehaviorConfig> WeaponBehaviorConfig;

private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;
	
	TObjectPtr<UCharacterInventoryComponent> InventoryComponentRef;
	TObjectPtr<UCharacterStateComponent> StateComponentRef;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FHandedItemData LeftHandItemData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FHandedItemData RightHandItemData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	AItemActor* LeftHandItemActor = nullptr;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	AItemActor* RightHandItemActor = nullptr;

	FReloadingData ReloadingData;

	FTimerHandle ReloadTimerHandle;
	
public:
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
	
protected:
	void EquipOrUnequipSlot(const FString& SlotName, UItemObject* IncomingItem);
	void UnEquipSlot(const FString& SlotName);
	
	bool ArmLeftHand(UItemObject* ItemObject);
	bool ArmRightHand(UItemObject* ItemObject);
	void DisarmLeftHand();
	void DisarmRightHand();
	
	void ArmHand(FHandedItemData& HandedItemData, AItemActor*& ReplicatedItemActor, const FName& SocketName, UItemObject* ItemObject);
	void DisarmHand(FHandedItemData& HandedItemData, AItemActor*& ReplicatedItemActor);
	
	virtual AItemActor* SpawnWeapon(USceneComponent* AttachmentComponent, UItemObject* ItemObject, FName SocketName) const;
	
public:
	void OnSlotEquipped(const FUpdatedSlotData& SlotData, FString SlotName);
	
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

	const FItemBehavior* GetItemBehavior(const FName& ItemScriptName) const;
	
	FORCEINLINE bool IsLeftItemObjectValid() const;
	FORCEINLINE bool IsRightItemObjectValid() const;
	
	FORCEINLINE bool IsLeftItemActorValid() const;
	FORCEINLINE bool IsRightItemActorValid() const;

	UCharacterInventoryComponent* GetCharacterInventory() const;
};
