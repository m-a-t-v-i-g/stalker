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
	FString KnifeSlotName = "Knife";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString MainSlotName = "Main";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString SecondarySlotName = "Secondary";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString GrenadeSlotName = "Grenade";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString BinocularSlotName = "Binocular";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString BoltSlotName = "Bolt";
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString DetectorSlotName = "Detector";

private:
	TObjectPtr<class AStalkerCharacter> StalkerCharacter;
	
	TObjectPtr<UCharacterInventoryComponent> CharacterInventory;
	
public:
	void OnKnifeSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnMainSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnSecondarySlotChanged(UItemObject* ItemObject, bool bModified);
	void OnGrenadeSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnBinocularSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnBoltSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnDetectorSlotChanged(UItemObject* ItemObject, bool bModified);
};
