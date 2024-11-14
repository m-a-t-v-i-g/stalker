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
	
	UItemObject* ItemObject = nullptr;

	AItemActor* ItemActor = nullptr;

	EMouseButtonReaction MouseButtonReaction = EMouseButtonReaction::None;
	
	FHandedItemData() = default;
	
	FHandedItemData(UItemObject* ItemObj, AItemActor* ItemAct, EMouseButtonReaction MouseReaction) :
		ItemObject(ItemObj), ItemActor(ItemAct), MouseButtonReaction(MouseReaction)
	{
	}

	void Clear()
	{
		ItemObject = nullptr;
		ItemActor = nullptr;
		MouseButtonReaction = EMouseButtonReaction::None;
	}

	bool IsValid() const
	{
		return ItemObject != nullptr;
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
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<const UItemBehaviorConfig> WeaponBehaviorConfig;

private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;
	
	TObjectPtr<UCharacterInventoryComponent> InventoryComponentRef;
	TObjectPtr<UCharacterStateComponent> StateComponentRef;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<UItemObject> LeftHandItemObject;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<UItemObject> RightHandItemObject;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AItemActor> LeftHandItemActor;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	TObjectPtr<AItemActor> RightHandItemActor;

	FReloadingData ReloadingData;

	FTimerHandle ReloadTimer;
	
public:
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
	void EquipOrUnEquipSlot(const FString& SlotName, UItemObject* IncomingItem);
	void UnEquipSlot(const FString& SlotName);
	
	bool ArmLeftHand(const FString& SlotName, UItemObject* ItemObject);
	bool ArmRightHand(const FString& SlotName, UItemObject* ItemObject);
	void DisarmLeftHand();
	void DisarmRightHand();
	
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
	
	FORCEINLINE bool IsLeftItemObjectValid() const;
	FORCEINLINE bool IsRightItemObjectValid() const;
	
	FORCEINLINE bool IsLeftItemActorValid() const;
	FORCEINLINE bool IsRightItemActorValid() const;

	UCharacterInventoryComponent* GetCharacterInventory() const;
};
