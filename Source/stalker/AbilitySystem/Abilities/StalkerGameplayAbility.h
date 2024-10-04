// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility_CharacterJump.h"
#include "StalkerGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class EAbilityActivationPolicy : uint8
{
	OnInputTriggered,
	WhileInputActive,
	OnSpawn
};

UCLASS()
class STALKER_API UStalkerGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Activation")
	EAbilityActivationPolicy ActivationPolicy;

public:
	EAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
};
