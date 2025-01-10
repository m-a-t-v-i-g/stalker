// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBehaviorSet.h"

const FWeaponBehavior* UItemBehaviorSet::GetWeaponBehavior(const FName& ItemScriptName) const
{
	return Weapons.Find(ItemScriptName);
}

const FArmorBehavior* UItemBehaviorSet::GetArmorBehavior(const FName& ItemScriptName) const
{
	return Armors.Find(ItemScriptName);
}
