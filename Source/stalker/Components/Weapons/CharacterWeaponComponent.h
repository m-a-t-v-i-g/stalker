// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "CharacterWeaponComponent.generated.h"

class UCharacterInventoryComponent;

UCLASS(meta=(BlueprintSpawnableComponent))
class STALKER_API UCharacterWeaponComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	UCharacterWeaponComponent();

	virtual void PreInitializeWeapon() override;
	virtual void PostInitializeWeapon() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString MainSlotName = "Main";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString SecondarySlotName = "Secondary";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString DetectorSlotName = "Detector";

private:
	TObjectPtr<UCharacterInventoryComponent> CharacterInventory;
	
public:
	void OnMainSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnSecondarySlotChanged(UItemObject* ItemObject, bool bModified);
	void OnDetectorSlotChanged(UItemObject* ItemObject, bool bModified);
};
