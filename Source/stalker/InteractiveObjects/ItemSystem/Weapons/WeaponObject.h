// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "WeaponObject.generated.h"

class AWeaponActor;

UCLASS()
class STALKER_API UWeaponDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bMelee = false;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (EditCondition = "!bMelee"))
	TArray<TSubclassOf<UAmmoObject>> AmmoClasses;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (EditCondition = "!bMelee", ClampMin = "1"))
	int MagSize = 0;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (EditCondition = "!bMelee", ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bAutomatic = false;
};

UCLASS()
class STALKER_API UWeaponPredictedData : public UItemPredictedData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0"))
	int Rounds = 0;
};

UCLASS()
class STALKER_API UWeaponInstance : public UItemInstance
{
	GENERATED_BODY()

public:
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemPredictedData* PredictedData) override;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<TSubclassOf<UAmmoObject>> AmmoClasses;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0"))
	int Rounds = 0;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	bool bAutomatic = false;
};

UCLASS()
class STALKER_API UWeaponObject : public UItemObject
{
	GENERATED_BODY()

protected:
	virtual void Use_Implementation(UObject* Source) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void InitItem(const uint32 ItemId, const UItemObject* ItemObject) override;
	
	virtual void OnBindItem() override;

	virtual bool IsSimilar(const UItemObject* OtherItemObject) const override;
	
protected:
	UPROPERTY(Replicated)
	FWeaponParams WeaponParams;

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponAttackStart();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnWeaponAttackStop();
	
public:
	virtual void ReloadAmmo();
	
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
