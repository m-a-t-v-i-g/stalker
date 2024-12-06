// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSystemCore.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class UItemObject;
class AItemActor;

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

protected:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<FOutfitSlot> WeaponSlots;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAllowUnarmedAttack = false;

public:
	virtual void ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName);

	virtual bool Attack();
	virtual void OnAttack();

	virtual bool CanAttack() const;
	virtual bool IsArmed() const;
	
	FORCEINLINE FOutfitSlot* FindWeaponSlot(const FString& SlotName);

	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;
};
