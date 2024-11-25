// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageSystemCore.h"

float UDamageSystemCore::CalculateProjectileDamage(float BaseDamage, float DamageMultiplier)
{
	return BaseDamage * DamageMultiplier;
}
