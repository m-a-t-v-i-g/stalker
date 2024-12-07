// Fill out your copyright notice in the Description page of Project Settings.

#include "ArmorBehaviorConfig.h"

const FArmorBehavior* UArmorBehaviorConfig::GetHandBehavior(const FName& ItemScriptName) const
{
	return ItemsMap.Find(ItemScriptName);
}
