// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

USTRUCT()
struct FEquippedHandEntry
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UItemObject> ItemObject;

	UPROPERTY(VisibleInstanceOnly, NotReplicated)
	TObjectPtr<const UAbilitySet> AbilitySet;

	UPROPERTY(NotReplicated)
	TArray<FGameplayAbilitySpecHandle> Abilities;

	const FHandItemBehavior* WeaponBehavior = nullptr;
	
	FEquippedHandEntry() {}
	
	FEquippedHandEntry(UItemObject* ItemObj, AItemActor* ItemAct,
	                   const FHandItemBehavior& ItemBeh) : ItemObject(ItemObj), WeaponBehavior(&ItemBeh)
	{
	}

	void Clear()
	{
		ItemObject = nullptr;
		AbilitySet = nullptr;
		WeaponBehavior = nullptr;
		Abilities.Empty();
	}

	bool IsValid() const
	{
		return ItemObject != nullptr;
	}
};

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterWeaponComponent : public UCharacterOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterWeaponComponent(const FObjectInitializer& ObjectInitializer);

	TMulticastDelegate<void()> OnFireStartDelegate;
	TMulticastDelegate<void()> OnFireStopDelegate;
	TMulticastDelegate<void()> OnAimingStartDelegate;
	TMulticastDelegate<void()> OnAimingStopDelegate;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnCharacterDead() override;

	virtual void TickItemAtHand(float DeltaTime);
	
	void ToggleSlot(uint8 SlotIndex);
	
	UFUNCTION(Server, Reliable)
	void ServerToggleSlot(int8 SlotIndex);

	UFUNCTION()
	void StartAiming();

	UFUNCTION()
	void StopAiming();

	void UpdateSpreadAngleMultipliers(float DeltaTime);
	float GetCalculatedSpreadAngle() const;

	const FHandItemBehavior* GetHandItemBehavior(const FName& ItemScriptName) const;
	
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
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* InItem) override;
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem) override;
	
	void ShowOrHideItem(UItemObject* InItem);

	void ArmHand(FEquippedHandEntry& HandedItemData, AItemActor*& ReplicatedItemActor, const FName& SocketName,
	             UItemObject* ItemObject);
	void DisarmHand(FEquippedHandEntry& HandedItemData, AItemActor*& ReplicatedItemActor);

	void AddWeaponRecoil();
	
	UFUNCTION()
	void OnWeaponFireStart();

	UFUNCTION()
	void OnWeaponFireStop();

	bool BindWeaponObject(UWeaponObject* WeaponObject);
	bool UnbindWeaponObject(UWeaponObject* WeaponObject);
	
	virtual AItemActor* SpawnItemAtHand(USceneComponent* AttachmentComponent, UItemObject* ItemObject, FName SocketName);
	
private:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FEquippedHandEntry LeftHandItemData;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_RightHandItemData", Category = "Weapon")
	FEquippedHandEntry RightHandItemData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	AItemActor* LeftHandItemActor = nullptr;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	AItemActor* RightHandItemActor = nullptr;

	float CurrentSpreadAngleMultiplier = 1.0f;

	float StandingStillSpeedThreshold = 0.0f;
	float StandingStillToMovingSpeedRange = 750.0f;
	
	float SpreadAngleMultiplier_StandingStill = 0.8f;
	float SpreadAngleMultiplier_SpeedValue = 3.0f;
	float SpreadAngleMultiplier_Crouching = 0.8f;
	float SpreadAngleMultiplier_JumpingOrFalling = 2.5f;

	float TransitionRate_StandingStill = 5.0f;
	float TransitionRate_Crouching = 5.0f;
	float TransitionRate_JumpingOrFalling = 5.0f;

	float StandingStillMultiplier = 1.0f;
	float CrouchingMultiplier = 1.0f;
	float JumpOrFallMultiplier = 1.0f;

	float TargetArmLength = 0.0f;
	
	UFUNCTION()
	void OnRep_RightHandItemData(const FEquippedHandEntry& PrevHandEntry);

	UFUNCTION()
	void OnRep_RightHandItemActor(AItemActor* PrevItemActor);
};
