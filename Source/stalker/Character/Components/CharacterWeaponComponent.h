// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "CharacterOutfitComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "Ammo/AmmoObject.h"
#include "Data/ItemBehaviorSet.h"
#include "CharacterWeaponComponent.generated.h"

struct FEquipmentSlotChangeData;
class UCharacterInventoryComponent;
class UCharacterStateComponent;
class UWeaponObject;
class UAbilitySet;
class AStalkerCharacter;

DECLARE_MULTICAST_DELEGATE(FCharacterFireDelegate);
DECLARE_MULTICAST_DELEGATE(FCharacterAimingDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterStartReloadSignature, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterStopReloadSignature, bool);

USTRUCT()
struct FEquippedWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UItemObject> ItemObject;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AItemActor> ItemActor;

	UPROPERTY(VisibleInstanceOnly)
	FWeaponBehavior WeaponBehavior;
	
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<const UAbilitySet> AbilitySet;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> Abilities;

	FEquippedWeaponData() {}
	
	FEquippedWeaponData(UItemObject* ItemObj, AItemActor* ItemAct, const FWeaponBehavior& ItemBeh) : ItemObject(ItemObj),
		ItemActor(ItemAct), WeaponBehavior(ItemBeh)
	{
	}

	void Clear()
	{
		ItemObject = nullptr;
		ItemActor = nullptr;
		AbilitySet = nullptr;
		WeaponBehavior.Clear();
		Abilities.Empty();
	}

	bool IsValid() const
	{
		return ItemObject != nullptr && ItemActor != nullptr;
	}
};

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterWeaponComponent : public UCharacterOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterWeaponComponent(const FObjectInitializer& ObjectInitializer);

	FCharacterFireDelegate OnFireStart;
	FCharacterFireDelegate OnFireStop;
	FCharacterAimingDelegate OnAimingStart;
	FCharacterAimingDelegate OnAimingStop;
	FOnCharacterStartReloadSignature OnReloadStart;
	FOnCharacterStopReloadSignature OnReloadStop;
	TMulticastDelegate<void(ECharacterOverlayState)> OnOverlayChanged;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnCharacterDead() override;
	
	void ToggleSlot(uint8 SlotIndex);
	
	UFUNCTION(Server, Reliable)
	void ServerToggleSlot(int8 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void StartAiming();

	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void StopAiming();
	
	const FWeaponBehavior* GetWeaponBehavior(const FName& ItemScriptName) const;
	
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
	virtual void SetupOutfitComponent(AStalkerCharacter* InCharacter) override;
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem) override;
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem) override;
	
	void EquipOrUnequipSlot(const FString& SlotName, UItemObject* IncomingItem);

	void ArmHand(FEquippedWeaponData& HandedItemData, AItemActor*& ReplicatedItemActor, const FName& SocketName,
	             UItemObject* ItemObject);
	void DisarmHand(FEquippedWeaponData& HandedItemData, AItemActor*& ReplicatedItemActor);

	UFUNCTION()
	void OnAttackStart();

	UFUNCTION()
	void OnAttackStop();
	
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
};
