// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemsLibrary.h"
#include "WeaponComponent.generated.h"

class AItemActor;
class UItemObject;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnWeaponSlotSignature, const FString&, int8, UItemObject*);

USTRUCT(Blueprintable)
struct FWeaponSlotSpec
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Weapon Slot")
	FString SlotName = "Default";
	
	UPROPERTY(EditAnywhere, Category = "Weapon Slot")
	FName AttachmentSocketName = "Default";
	
	const FString& GetSlotName() const { return SlotName; }
};

USTRUCT(Blueprintable)
struct FWeaponSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleInstanceOnly)
	uint8 Handle = 0;
	
	UPROPERTY(VisibleInstanceOnly)
	UItemObject* ArmedItemObject = nullptr;
	
	FWeaponSlotSpec* WeaponSlotPtr = nullptr;
	
	FWeaponSlot()
	{
	}

	FWeaponSlot(uint8 Index, FWeaponSlotSpec& SlotSpec)
	{
		Handle = Index;
		WeaponSlotPtr = &SlotSpec;
	}

	const FString& GetSlotName() const { return WeaponSlotPtr->SlotName; }
	const FName& GetAttachmentSocketName() const { return WeaponSlotPtr->AttachmentSocketName; }

	bool IsArmed() const { return ArmedItemObject != nullptr; }

	friend uint8 GetTypeHash(const FWeaponSlot& Handle)
	{
		return GetTypeHash(Handle.Handle);
	}
};

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

private:
	UPROPERTY(EditDefaultsOnly, DisplayName = "Weapon Slots", Category = "Weapon")
	TArray<FWeaponSlotSpec> WeaponSlotSpecs;

protected:
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAllowUnarmedAttack = false;

	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TMap<uint8, bool> ActiveSlots;
	
public:
	FOnWeaponSlotSignature OnSlotActivated;
	FOnWeaponSlotSignature OnSlotDeactivated;
	
	virtual void PreInitializeWeapon();
	virtual void PostInitializeWeapon();

	UFUNCTION(Server, Unreliable)
	void ServerActivateSlot(int8 SlotIndex);

	UFUNCTION(Server, Unreliable)
	void ServerDeactivateSlot(int8 SlotIndex);
	
	virtual void ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName);

	virtual bool Attack();
	virtual void OnAttack();

	virtual bool CanAttack() const;

	FORCEINLINE FWeaponSlot* FindWeaponSlot(const FString& SlotName);
};
