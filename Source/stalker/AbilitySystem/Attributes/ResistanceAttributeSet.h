// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ResistanceAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class STALKER_API UResistanceAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_BulletResistance", Category = "Resistance")
	FGameplayAttributeData BulletResistance;
	ATTRIBUTE_ACCESSORS(UResistanceAttributeSet, BulletResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_BlastResistance", Category = "Resistance")
	FGameplayAttributeData BlastResistance;
	ATTRIBUTE_ACCESSORS(UResistanceAttributeSet, BlastResistance);

protected:
	UFUNCTION()
	virtual void OnRep_BulletResistance(const FGameplayAttributeData& OldBulletResistance);

	UFUNCTION()
	virtual void OnRep_BlastResistance(const FGameplayAttributeData& OldBlastResistance);
};
