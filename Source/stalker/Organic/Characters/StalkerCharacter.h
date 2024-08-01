// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

class UWeaponComponent;
class UItemObject;

UCLASS()
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;
	
	static FName WeaponComponentName;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString ArmorSlotName = "Armor";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString MainSlotName = "Main";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString SecondarySlotName = "Secondary";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString DetectorSlotName = "Detector";

	UPROPERTY(EditAnywhere, Category = "Character")
	TObjectPtr<UWeaponComponent> CharacterWeapon;
	
public:
	void OnArmorSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnMainSlotChanged(UItemObject* ItemObject, bool bModified);
	void OnSecondarySlotChanged(UItemObject* ItemObject, bool bModified);
	void OnDetectorSlotChanged(UItemObject* ItemObject, bool bModified);
};
