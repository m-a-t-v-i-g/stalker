// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "WeaponActor.generated.h"

struct FWeaponParams;
class UWeaponObject;

DECLARE_DELEGATE(FOnWeaponAttackDelegate);

UCLASS()
class STALKER_API AWeaponActor : public AItemActor
{
	GENERATED_BODY()

public:
	AWeaponActor();

private:
	const FWeaponParams* WeaponParams = nullptr;
	
public:
	FOnWeaponAttackDelegate OnWeaponStartAttack;
	FOnWeaponAttackDelegate OnWeaponStopAttack;

private:
	FTimerHandle CanAttackTimer;
	FTimerHandle RepeatAttackTimer;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bInFireRate = true;

	bool bHoldTrigger = false;
	
public:
	virtual void OnInitializeItem() override;

	void StartAttack();
	void CallAttack();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMakeAttackVisual();
	
	void StopAttack();

	virtual bool CheckAttackAvailability() const;
	
	bool IsAmmoAvailable(const UClass* AmmoClass) const;

protected:
	void SetInFireRateTimer();
	void SetRepeatFireTimer();

	float GetDefaultFireRate() const;
	virtual float CalculateFireRate() const;
};
