// Fill out your copyright notice in the Description page of Project Settings.

#include "AmmoObject.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalObjects/ProjectileBase.h"

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
		// Data
		
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
		// Data
	}
}

const UAmmoDefinition* UAmmoObject::GetAmmoDefinition() const
{
	return Cast<UAmmoDefinition>(GetDefinition());
}

UClass* UAmmoObject::GetProjectileClass() const
{
	return GetAmmoDefinition()->ProjectileClass;
}
