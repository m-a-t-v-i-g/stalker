// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemsLibrary.h"
#include "WeaponComponent.generated.h"

class AItemActor;
class UItemObject;

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
	AItemActor* ArmedItemActor = nullptr;
	
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

	bool IsArmed() const { return ArmedItemActor != nullptr && ArmedItemObject != nullptr; }
	
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
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;
	
private:
	UPROPERTY(EditDefaultsOnly, DisplayName = "Weapon Slots", Category = "Weapon")
	TArray<FWeaponSlotSpec> WeaponSlotSpecs;

	uint8 ActiveSlot = 0;
	
public:
	virtual void PreInitializeWeapon();
	virtual void PostInitializeWeapon();
	
	bool ActivateSlot(const FString& SlotName);
	bool ActivateSlot(uint8 SlotIndex);

	void ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	void DisarmSlot(const FString& SlotName);

	void Attack();

protected:
	AItemActor* SpawnWeapon(const FWeaponSlot* WeaponSlot, const UItemObject* ItemObject) const;
	
public:
	FORCEINLINE FWeaponSlot* FindWeaponSlot(const FString& SlotName);
};
