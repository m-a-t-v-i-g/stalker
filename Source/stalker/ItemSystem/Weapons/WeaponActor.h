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
	AWeaponActor(const FObjectInitializer& ObjectInitializer);

	virtual void OnBindItem() override;
	virtual void OnUnbindItem(UItemObject* PrevItemObject) override;

	UFUNCTION()
	virtual void OnFireStart();

	UFUNCTION()
	virtual void OnFireStop();

	FORCEINLINE UWeaponObject* GetWeaponObject() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Muzzle;
};
