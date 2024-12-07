// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBehaviorConfig.h"

const FWeaponBehavior* UItemBehaviorConfig::GetWeaponBehavior(const FName& ItemScriptName) const
{
	return Weapons.Find(ItemScriptName);
}

const FArmorBehavior* UItemBehaviorConfig::GetArmorBehavior(const FName& ItemScriptName) const
{
	return Armors.Find(ItemScriptName);
}
