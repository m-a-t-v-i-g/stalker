// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "AmmoObject.generated.h"

class UGameplayEffect;
class ABulletBase;

USTRUCT()
struct FAmmoInstanceData
{
	GENERATED_USTRUCT_BODY()
};

USTRUCT()
struct FAmmoDamageData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;
	
	/** Basic bullet damage without any multiplying values or effects. */
	UPROPERTY(EditAnywhere, Category = "Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 0.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Damage")
	TArray<TSubclassOf<UGameplayEffect>> DamageEffects;
};

UCLASS()
class STALKER_API UAmmoDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<ABulletBase> BulletClass;

	UPROPERTY(EditAnywhere, Category = "Ammo", meta = (ShowOnlyInnerProperties))
	FAmmoDamageData DamageData;
};

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UAmmoPredictedData : public UItemPredictedData
{
	GENERATED_BODY()
};

UCLASS()
class STALKER_API UAmmoInstance : public UItemInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Ammo")
	FAmmoInstanceData AmmoData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Ammo", meta = (ShowOnlyInnerProperties))
	FAmmoDamageData DamageData;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
								 const UItemPredictedData* PredictedData) override;
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
								 const UItemInstance* Instance) override;
};

UCLASS(EditInlineNew)
class STALKER_API UAmmoObject : public UItemObject
{
	GENERATED_BODY()

public:
	FORCEINLINE const UAmmoDefinition* GetAmmoDefinition() const;
	FORCEINLINE UClass* GetBulletClass() const;
	FORCEINLINE FAmmoDamageData GetDamageData() const;
	FORCEINLINE UClass* GetDamageType() const;
	FORCEINLINE TArray<TSubclassOf<UGameplayEffect>> GetDamageEffects() const;
};
