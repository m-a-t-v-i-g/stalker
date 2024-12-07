// Fill out your copyright notice in the Description page of Project Settings.

#include "HandBehaviorConfig.h"

const FHandBehavior* UHandBehaviorConfig::GetHandBehavior(const FName& ItemScriptName) const
{
	return ItemsMap.Find(ItemScriptName);
}
