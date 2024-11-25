// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "AmmoObject.generated.h"

class AProjectileBase;

USTRUCT()
struct FAmmoInstanceData
{
	GENERATED_USTRUCT_BODY()
};

UCLASS()
class STALKER_API UAmmoDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<AProjectileBase> ProjectileClass;
};

UCLASS(EditInlineNew, DefaultToInstanced)
class STALKER_API UAmmoPredictedData : public UItemPredictedData
{
	GENERATED_BODY()

public:
};

UCLASS()
class STALKER_API UAmmoInstance : public UItemInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Weapon")
	FAmmoInstanceData AmmoData;

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
	FORCEINLINE UClass* GetProjectileClass() const;
};
