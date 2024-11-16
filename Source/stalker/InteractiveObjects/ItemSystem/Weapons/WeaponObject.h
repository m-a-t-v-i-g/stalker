// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "WeaponObject.generated.h"

class AWeaponActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponAttackSignature);

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
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = "0"))
	int Rounds = 0;
};

UCLASS()
class STALKER_API UWeaponInstance : public UItemInstance
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<TSubclassOf<UAmmoObject>> AmmoClasses;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "1"))
	int MagSize = 0;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0"))
	int Rounds = 0;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	bool bAutomatic = false;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemPredictedData* PredictedData) override;
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemInstance* Instance) override;
};

UCLASS()
class STALKER_API UWeaponObject : public UItemObject
{
	GENERATED_BODY()

public:
	FOnWeaponAttackSignature OnAttackStart;
	FOnWeaponAttackSignature OnAttackStop;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnBindItemActor() override;
	virtual void OnUnbindItemActor(AItemActor* PrevItemActor) override;

	virtual bool IsSimilar(const UItemObject* OtherItemObject) const override;
	
	void StartAttack();
	void CallAttack();
	void StopAttack();

	void StartAlternative();
	void StopAlternative();
	
	virtual void ReloadAmmo();
	
	virtual void IncreaseAmmo(int Amount);
	virtual void DecreaseAmmo();

	virtual int CalculateRequiredAmmoCount() const;
	virtual float CalculateFireRate() const;
	
	FORCEINLINE bool IsAutomatic() const;
	FORCEINLINE bool IsMagFull() const;
	FORCEINLINE bool IsMagEmpty() const;
	FORCEINLINE bool CanAttack() const;
	
	FORCEINLINE int GetMagSize() const;
	FORCEINLINE float GetReloadTime() const;
	FORCEINLINE float GetDefaultFireRate() const;
	
protected:
	virtual void Use_Implementation(UObject* Source) override;

	void SetSingleFireTimer();
	void SetRepetitiveFireTimer();

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnAttack();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnStopAttack();
	
private:
	FTimerHandle CanAttackTimer;
	FTimerHandle RepeatAttackTimer;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bInFireRate = true;

	bool bHoldTrigger = false;
};
