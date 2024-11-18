// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "WeaponObject.generated.h"

class UAmmoDefinition;
class UAmmoObject;
class AWeaponActor;

USTRUCT()
struct FWeaponInstanceData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<const UAmmoDefinition*> AmmoClasses;

	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "1"))
	int MagSize = 0;

	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<UAmmoObject*> Rounds;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TWeakObjectPtr<const UAmmoDefinition> CurrentAmmoClass;

	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;

	UPROPERTY(EditInstanceOnly, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;

	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	bool bAutomatic = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponAttackSignature);

UCLASS()
class STALKER_API UWeaponDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bMelee = false;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (EditCondition = "!bMelee"))
	TArray<const UAmmoDefinition*> AmmoClasses;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (EditCondition = "!bMelee", ClampMin = "1"))
	int MagSize = 0;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (EditCondition = "!bMelee", ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bAutomatic = false;
};

USTRUCT()
struct FAmmoStartingData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Rounds")
	TObjectPtr<const UAmmoDefinition> Definition;
	
	UPROPERTY(EditAnywhere, Category = "Rounds", meta = (ClampMin = "0"))
	int Count = 0;
};

UCLASS()
class STALKER_API UWeaponPredictedData : public UItemPredictedData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<FAmmoStartingData> Ammo;
};

UCLASS()
class STALKER_API UWeaponInstance : public UItemInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FWeaponInstanceData WeaponData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemPredictedData* PredictedData) override;
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemInstance* Instance) override;
	virtual void SetupRounds(const UItemDefinition* Definition, const UItemPredictedData* PredictedData);
};

UCLASS()
class STALKER_API UWeaponObject : public UItemObject
{
	GENERATED_BODY()

public:
	FOnWeaponAttackSignature OnAttackStart;
	FOnWeaponAttackSignature OnAttackStop;

#pragma region Replication
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
#pragma endregion Replication

#pragma region Behavior
	
	virtual void Use_Implementation(UObject* Source) override;

	virtual void OnBindItemActor() override;
	virtual void OnUnbindItemActor(AItemActor* PrevItemActor) override;

	virtual bool IsSimilar(const UItemObject* OtherItemObject) const override;
	
	void StartAttack();
	void CallAttack();
	void StopAttack();

	void StartAlternative();
	void StopAlternative();
	
	virtual void IncreaseAmmo(UAmmoObject* AmmoObject, int Amount);
	virtual void DecreaseAmmo();

	virtual int CalculateRequiredAmmoCount() const;
	virtual float CalculateFireRate() const;
	
	FORCEINLINE bool IsMagFull() const;
	FORCEINLINE bool IsMagEmpty() const;
	FORCEINLINE bool CanAttack() const;
	
#pragma endregion Behavior
	
	FORCEINLINE TArray<const UAmmoDefinition*> GetAmmoClasses() const;
	FORCEINLINE int GetMagSize() const;
	FORCEINLINE TArray<UAmmoObject*> GetRounds() const;
	FORCEINLINE const UAmmoDefinition* GetCurrentAmmoClass() const;
	FORCEINLINE float GetReloadTime() const;
	FORCEINLINE float GetDefaultFireRate() const;
	FORCEINLINE bool IsAutomatic() const;
	
	FORCEINLINE AWeaponActor* GetWeaponActor() const;
	FORCEINLINE UWeaponInstance* GetWeaponInstance() const;

protected:
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

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	bool bHoldTrigger = false;
};
