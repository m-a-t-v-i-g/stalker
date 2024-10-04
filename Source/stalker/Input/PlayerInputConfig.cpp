// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerInputConfig.h"

UPlayerInputConfig::UPlayerInputConfig(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

const UInputAction* UPlayerInputConfig::FindNativeAction(const FGameplayTag& InputTag) const
{
	const UInputAction* FoundAction = nullptr;
	for (const FPlayerInputInfo& Action : NativeActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			FoundAction = Action.InputAction;
		}
	}
	return FoundAction;
}

const UInputAction* UPlayerInputConfig::FindAbilityAction(const FGameplayTag& InputTag) const
{
	const UInputAction* FoundAction = nullptr;
	for (const FPlayerInputInfo& Action : AbilityActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			FoundAction = Action.InputAction;
		}
	}
	return FoundAction;
}
