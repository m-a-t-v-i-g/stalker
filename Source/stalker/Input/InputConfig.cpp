// Fill out your copyright notice in the Description page of Project Settings.

#include "InputConfig.h"

UInputConfig::UInputConfig(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

const UInputAction* UInputConfig::FindNativeAction(const FGameplayTag& InputTag) const
{
	const UInputAction* FoundAction = nullptr;
	for (const FInputConfigData& Action : NativeActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			FoundAction = Action.InputAction;
		}
	}
	return FoundAction;
}

const UInputAction* UInputConfig::FindAbilityAction(const FGameplayTag& InputTag) const
{
	const UInputAction* FoundAction = nullptr;
	for (const FInputConfigData& Action : AbilityActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			FoundAction = Action.InputAction;
		}
	}
	return FoundAction;
}
