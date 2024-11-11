// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class UItemObject;
class AItemActor;

USTRUCT(Blueprintable)
struct FWeaponSlot
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Weapon Slot")
	FString SlotName = "Default";
	
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon Slot")
	UItemObject* ArmedObject = nullptr;
	
	FWeaponSlot()
	{
	}

	const FString& GetSlotName() const
	{
		return SlotName;
	}
	
	bool IsArmed() const
	{
		return ArmedObject != nullptr;
	}
};

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	virtual void SetupWeaponComponent();

protected:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<FWeaponSlot> WeaponSlots;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAllowUnarmedAttack = false;

public:
	virtual void ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName);

	virtual bool Attack();
	virtual void OnAttack();

	virtual bool CanAttack() const;
	virtual bool IsArmed() const;
	
	FORCEINLINE FWeaponSlot* FindWeaponSlot(const FString& SlotName);

	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;
};
