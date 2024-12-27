// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "WeaponObject.h"
#include "WeaponActor.generated.h"

struct FWeaponParams;
class UWeaponObject;
class ABulletBase;

UCLASS()
class STALKER_API AWeaponActor : public AItemActor
{
	GENERATED_BODY()

public:
	AWeaponActor();

	virtual void OnBindItem() override;
	virtual void OnUnbindItem(UItemObject* PrevItemObject) override;

	UFUNCTION()
	virtual void OnStartAttack();

	virtual void MakeAttackVisual();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMakeAttackVisual();

	UFUNCTION()
	virtual void OnStopAttack();

	UFUNCTION()
	virtual void OnStartAlternative();

	UFUNCTION()
	virtual void OnStopAlternative();

	UWeaponObject* GetWeaponObject() const { return Cast<UWeaponObject>(GetItemObject()); }
	
protected:
	virtual void OnSetupBullet(ABulletBase* Bullet);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Muzzle;

	TObjectPtr<AController> InstigatorController;
	
	FTimerHandle CanAttackTimer;
	FTimerHandle RepeatAttackTimer;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bInFireRate = true;

	bool bHoldTrigger = false;
};
