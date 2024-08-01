// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterArmorComponent.generated.h"

class UItemObject;

UCLASS(ClassGroup = "Stalker", meta=(BlueprintSpawnableComponent))
class STALKER_API UCharacterArmorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterArmorComponent();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Armor")
	FString ArmorSlotName = "Armor";

public:
	virtual void PreInitializeArmor();
	virtual void PostInitializeArmor();

	void OnArmorSlotChanged(UItemObject* ItemObject, bool bModified);
};
