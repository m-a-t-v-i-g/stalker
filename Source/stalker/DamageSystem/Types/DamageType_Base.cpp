// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageType_Base.h"
#include "GameplayEffect.h"

UClass* UDamageType_Base::GetDamageEffect() const
{
	return DamageEffect;
}

FGameplayTag UDamageType_Base::GetDamageTag() const
{
	return DamageTag;
}
