// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBehaviorConfig.h"

const FItemBehavior* UItemBehaviorConfig::GetItemBehavior(const FName& ScriptName) const
{
	return ItemsMap.Find(ScriptName);
}
