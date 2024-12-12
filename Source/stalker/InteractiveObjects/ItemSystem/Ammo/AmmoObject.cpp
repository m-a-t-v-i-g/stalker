// Fill out your copyright notice in the Description page of Project Settings.

#include "AmmoObject.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalObjects/BulletBase.h"

void UAmmoInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, AmmoData, COND_OwnerOnly);
}

void UAmmoInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                    const UItemPredictedData* PredictedData)
{
	Super::SetupProperties(NewItemId, Definition, PredictedData);

	if (auto AmmoDefinition = Cast<UAmmoDefinition>(Definition))
	{
		DamageData.BaseDamage = AmmoDefinition->DamageData.BaseDamage;
		DamageData.DamageType = AmmoDefinition->DamageData.DamageType;
		DamageData.DamageEffect = AmmoDefinition->DamageData.DamageEffect;
		
		if (auto AmmoPredictedData = Cast<UAmmoPredictedData>(PredictedData))
		{
			// Predicted data
		}
	}
}

void UAmmoInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	Super::SetupProperties(NewItemId, Definition, Instance);

	if (auto AmmoInstance = Cast<UAmmoInstance>(Instance))
	{
		DamageData.BaseDamage = AmmoInstance->DamageData.BaseDamage;
		DamageData.DamageType = AmmoInstance->DamageData.DamageType;
		DamageData.DamageEffect = AmmoInstance->DamageData.DamageEffect;
	}
}

const UAmmoDefinition* UAmmoObject::GetAmmoDefinition() const
{
	return Cast<UAmmoDefinition>(GetDefinition());
}

UClass* UAmmoObject::GetBulletClass() const
{
	return GetAmmoDefinition()->BulletClass;
}

FAmmoDamageData UAmmoObject::GetDamageData() const
{
	return GetAmmoDefinition()->DamageData;
}

UClass* UAmmoObject::GetDamageType() const
{
	return GetAmmoDefinition()->DamageData.DamageType;
}

UClass* UAmmoObject::GetDamageEffect() const
{
	return GetAmmoDefinition()->DamageData.DamageEffect;
}
