// Fill out your copyright notice in the Description page of Project Settings.

#include "AmmoObject.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalObjects/BulletBase.h"

void UAmmoInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, AmmoData,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, DamageData,	COND_OwnerOnly);
}

void UAmmoInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                    const UItemPredictedData* PredictedData)
{
	if (auto AmmoDefinition = Cast<UAmmoDefinition>(Definition))
	{
		DamageData.BaseDamage = AmmoDefinition->DamageData.BaseDamage;
		DamageData.DamageType = AmmoDefinition->DamageData.DamageType;
		
		if (auto AmmoPredictedData = Cast<UAmmoPredictedData>(PredictedData))
		{
			// Predicted data
		}
	}

	Super::SetupProperties(NewItemId, Definition, PredictedData);
}

void UAmmoInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	if (auto AmmoInstance = Cast<UAmmoInstance>(Instance))
	{
		DamageData.BaseDamage = AmmoInstance->DamageData.BaseDamage;
		DamageData.DamageType = AmmoInstance->DamageData.DamageType;
	}

	Super::SetupProperties(NewItemId, Definition, Instance);
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
