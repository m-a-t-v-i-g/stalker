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

public:
	UStalkerGameplayAbility(const FObjectInitializer& ObjectInitializer);
	
	AController* GetControllerFromActorInfo() const;
	
	EAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Activation")
	EAbilityActivationPolicy ActivationPolicy;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) override;
};
