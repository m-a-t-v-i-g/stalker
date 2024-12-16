// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/DamageType.h"
#include "DamageType_Base.generated.h"

class UGameplayEffect;

UCLASS()
class STALKER_API UDamageType_Base : public UDamageType
{
	GENERATED_BODY()

public:
	UClass* GetDamageEffect() const;
	
	FGameplayTag GetDamageTag() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Effects")
	FGameplayTag DamageTag;
};
