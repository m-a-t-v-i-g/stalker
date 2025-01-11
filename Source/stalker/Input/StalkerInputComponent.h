// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Input/InputConfig.h"
#include "StalkerInputComponent.generated.h"

class UInputConfig;

UCLASS(ClassGroup = "Stalker")
class STALKER_API UStalkerInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UStalkerInputComponent(const FObjectInitializer& ObjectInitializer);

	template <class UserClass, typename FuncType>
	void BindNativeAction(const UInputConfig* InputConfig, const FGameplayTag& InputTag,
	                      ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc,
	                        ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UStalkerInputComponent::BindNativeAction(const UInputConfig* InputConfig, const FGameplayTag& InputTag,
                                              ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeAction(InputTag))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UStalkerInputComponent::BindAbilityActions(const UInputConfig* InputConfig, UserClass* Object,
                                                PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc,
                                                TArray<uint32>& BindHandles)
{
	check(InputConfig);
	for (const FInputConfigData& Action : InputConfig->AbilityActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
