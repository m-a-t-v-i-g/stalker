// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBehaviorSet.h"

const FHandItemBehavior* UItemBehaviorSet::GetHandItemBehavior(const FName& ItemScriptName) const
{
	return HandItems.Find(ItemScriptName);
}

const FArmorBehavior* UItemBehaviorSet::GetArmorBehavior(const FName& ItemScriptName) const
{
	return Armors.Find(ItemScriptName);
}
