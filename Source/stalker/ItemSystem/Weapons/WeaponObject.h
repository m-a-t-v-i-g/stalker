// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "WeaponObject.generated.h"

class UNiagaraSystem;
class UAmmoDefinition;
class UAmmoObject;
class AWeaponActor;

USTRUCT()
struct FWeaponInstanceData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon")
	TArray<const UAmmoDefinition*> AmmoClasses;

	UPROPERTY(EditInstanceOnly, Category = "Weapon|Magazin", meta = (ClampMin = "1"))
	int MagSize = 0;

	UPROPERTY(EditInstanceOnly, Category = "Weapon|Magazin", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;

	UPROPERTY(EditInstanceOnly, Category = "Weapon|Magazin")
	TArray<UAmmoObject*> Rounds;

	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Magazin")
	TWeakObjectPtr<const UAmmoDefinition> CurrentAmmoClass;

	UPROPERTY(EditInstanceOnly, Category = "Weapon|Fire Params", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;

	UPROPERTY(EditInstanceOnly, Category = "Weapon|Fire Params", meta = (ClampMin = "0.0"))
	float RecoilAngle = 0.15f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Fire Params", meta = (ClampMin = "1.0"))
	float NastinessMultiplier = 1.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Fire Params", meta = (ClampMin = "1.0"))
	float BulletSpeedMultiplier = 1.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Fire Params", meta = (ClampMin = "1.0"))
	float BulletImpulseMultiplier = 1.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Weapon|Fire Params", meta = (ClampMin = "0.0"))
	float AdditiveImpulse = 1.0f;
};

USTRUCT()
struct FWeaponDamageData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Damage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 0.0f;

	bool operator==(const FWeaponDamageData& OtherDamageData) const
	{
		return FMath::IsNearlyEqual(DamageMultiplier, OtherDamageData.DamageMultiplier, 0.01f);
	}
};

USTRUCT()
struct FWeaponPredictedAmmo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Rounds")
	TObjectPtr<const UAmmoDefinition> Definition;
	
	UPROPERTY(EditAnywhere, Category = "Rounds", meta = (ClampMin = "0"))
	int Count = 0;
};

UCLASS()
class STALKER_API UWeaponDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<const UAmmoDefinition*> AmmoClasses;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Effects")
	TSoftClassPtr<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere, DisplayName = "Fire", Category = "Weapon|Sounds")
	TSoftObjectPtr<USoundBase> FireSound;
	
	UPROPERTY(EditAnywhere, DisplayName = "Muzzle", Category = "Weapon|VFX")
	TSoftObjectPtr<UNiagaraSystem> MuzzleEffect;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Magazin", meta = (ClampMin = "0"))
	int MagSize = 0;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Magazin", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params", meta = (ClampMin = "0.0", ForceUnits = "rpm"))
	float FireRate = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params", meta = (ClampMin = "0.0"))
	float RecoilAngle = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params", meta = (ClampMin = "1.0"))
	float NastinessMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params", meta = (ClampMin = "1.0"))
	float BulletSpeedMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params", meta = (ClampMin = "1.0"))
	float BulletImpulseMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params", meta = (ClampMin = "0.0"))
	float AdditiveImpulse = 100.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Fire Params")
	FWeaponDamageData DamageData;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Specification", meta = (ClampMin = "0.1"))
	float SpreadExponent = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Specification", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float SpreadRecoveryCooldownDelay = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Specification")
	FRuntimeFloatCurve HeatToSpreadCurve;

	UPROPERTY(EditAnywhere, Category = "Weapon|Specification")
	FRuntimeFloatCurve HeatToRecoilCurve;

	UPROPERTY(EditAnywhere, Category = "Weapon|Specification")
	FRuntimeFloatCurve HeatToHeatPerShotCurve;
	
	UPROPERTY(EditAnywhere, Category = "Weapon|Specification")
	FRuntimeFloatCurve HeatToCooldownPerSecondCurve;
};

UCLASS()
class STALKER_API UWeaponPredictedData : public UItemPredictedData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TArray<FWeaponPredictedAmmo> Ammo;
};

UCLASS()
class STALKER_API UWeaponInstance : public UItemInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FWeaponInstanceData WeaponData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon", meta = (ShowOnlyInnerProperties))
	FWeaponDamageData DamageData;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemPredictedData* PredictedData) override;
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
	                             const UItemInstance* Instance) override;
	virtual void SetupRounds(const UItemDefinition* Definition, const UItemPredictedData* PredictedData);
	virtual void SetupRounds(const UItemInstance* Instance);

	void DefineCurrentAmmoClass();
};

UCLASS()
class STALKER_API UWeaponObject : public UItemObject
{
	GENERATED_BODY()

public:
	TMulticastDelegate<void()> OnFireStartDelegate;
	TMulticastDelegate<void()> OnFireStopDelegate;
	TMulticastDelegate<void()> CancelAllActionsDelegate;

#pragma region Replication
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
#pragma endregion Replication

#pragma region Behavior
	
	virtual void Use_Implementation(UObject* Source) override;
	virtual void OnBindItemActor() override;
	virtual void OnUnbindItemActor(AItemActor* PrevItemActor) override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual bool IsCorrespondsTo(const UItemObject* OtherItemObject) const override;

	void MakeEquipped();
	void MakeUnequipped();
	
	void StartFire();
	void StopFire();

	void UpdateSpread(float DeltaSeconds);
	void AddSpread();
	
	void UpdateRecoil(float DeltaSeconds);

	virtual void IncreaseAmmo(UAmmoObject* AmmoObject, int Amount);
	virtual void DecreaseAmmo();

	virtual void CancelAllActions();

	virtual int CalculateRequiredAmmoCount() const;
	
	FORCEINLINE bool IsMagFull() const;
	FORCEINLINE bool IsMagEmpty() const;
	FORCEINLINE bool CanAttack() const;

	FORCEINLINE float GetSpreadAngle() const;
	FORCEINLINE float GetRecoilMultiplier() const;
	
#pragma endregion Behavior

	FORCEINLINE const UWeaponDefinition* GetWeaponDefinition() const;

	FORCEINLINE UClass* GetCameraShake() const;
	
	FORCEINLINE USoundBase* GetFireSound() const;
	FORCEINLINE UNiagaraSystem* GetMuzzleEffect() const;
	
	FORCEINLINE TArray<const UAmmoDefinition*> GetAmmoClasses() const;
	FORCEINLINE int GetDefaultMagSize() const;
	FORCEINLINE TArray<UAmmoObject*> GetRounds() const;
	FORCEINLINE uint16 GetRemainedRounds() const;
	FORCEINLINE const UAmmoDefinition* GetCurrentAmmoClass() const;
	FORCEINLINE float GetReloadTime() const;
	FORCEINLINE float GetDefaultReloadTime() const;
	FORCEINLINE float GetFireRate() const;
	FORCEINLINE float GetDefaultFireRate() const;
	FORCEINLINE float GetRecoilAngle() const;
	FORCEINLINE float GetDefaultRecoilAngle() const;
	FORCEINLINE float GetNastinessMultiplier() const;
	FORCEINLINE float GetDefaultNastinessMultiplier() const;
	FORCEINLINE float GetBulletSpeedMultiplier() const;
	FORCEINLINE float GetDefaultBulletSpeedMultiplier() const;
	FORCEINLINE float GetBulletImpulseMultiplier() const;
	FORCEINLINE float GetDefaultBulletImpulseMultiplier() const;
	FORCEINLINE float GetAdditiveImpulse() const;
	FORCEINLINE FWeaponDamageData GetDamageData() const;
	FORCEINLINE float GetSpreadExponent() const;
	FORCEINLINE float GetDefaultSpreadExponent() const;
	
	FORCEINLINE float GetSpreadRecoveryCooldownDelay() const;
	FORCEINLINE const FRichCurve* GetHeatToSpreadCurve() const;
	FORCEINLINE const FRichCurve* GetHeatToRecoilCurve() const;
	FORCEINLINE const FRichCurve* GetHeatToHeatPerShotCurve() const;
	FORCEINLINE const FRichCurve* GetHeatToCooldownPerSecondCurve() const;
	
	FORCEINLINE AWeaponActor* GetWeaponActor() const;
	FORCEINLINE UWeaponInstance* GetWeaponInstance() const;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnEquipped();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnUnequipped();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnFireStart();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnFireStop();
	
	virtual float CalculateReloadTime() const;
	virtual float CalculateFireRate() const;
	virtual float CalculateSpreadExponent() const;

private:
	float LastFireTime = 0.0;
	float CurrentHeat = 0.0f;
	float CurrentSpreadAngle = 0.0f;
	float CurrentRecoilMultiplier = 1.0f;

	void ComputeSpreadRange(float& MinSpread, float& MaxSpread);
	void ComputeHeatRange(float& MinHeat, float& MaxHeat);
	float ClampHeat(float NewHeat);
};
