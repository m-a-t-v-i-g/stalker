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

	FOnWeaponAttackDelegate OnWeaponStartAttack;
	FOnWeaponAttackDelegate OnWeaponStopAttack;

	virtual void OnBindItem() override;
	virtual void OnUnbindItem(UItemObject* PrevItemObject) override;

	UFUNCTION()
	void OnStartAttack();

	virtual void MakeAttackVisual();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMakeAttackVisual();

	UFUNCTION()
	void OnStopAttack();

	UFUNCTION()
	virtual void OnStartAlternative();

	UFUNCTION()
	virtual void OnStopAlternative();
	
protected:
	
private:
	FTimerHandle CanAttackTimer;
	FTimerHandle RepeatAttackTimer;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bInFireRate = true;

	bool bHoldTrigger = false;
};
