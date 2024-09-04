// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemObject.h"
#include "WeaponObject.generated.h"

class AWeaponActor;

UCLASS()
class STALKER_API UWeaponObject : public UItemObject
{
	GENERATED_BODY()

protected:
	virtual void Use_Implementation(UObject* Source) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void InitItem(const uint32 ItemId, const FDataTableRowHandle& RowHandle) override;
	virtual void InitItem(const uint32 ItemId, const UItemObject* ItemObject) override;
	
	virtual void OnBindItem() override;

	virtual bool IsSimilar(const UItemObject* OtherItemObject) const override;
	
protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Instance Data")
	FWeaponParams WeaponParams;

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponAttackStart();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponAttackStop();
	
public:
	virtual void IncreaseAmmo(int Amount);
	virtual void DecreaseAmmo();

	virtual int CalculateRequiredAmmoCount() const;
	
	FORCEINLINE bool IsAutomatic() const;
	FORCEINLINE bool IsMagFull() const;
	FORCEINLINE bool IsMagEmpty() const;
	FORCEINLINE bool CanAttack() const;
	
	FORCEINLINE int GetMagSize() const;
	
	FORCEINLINE const FWeaponParams& GetWeaponParams() const;
};
