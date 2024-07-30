// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

UCLASS()
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString ArmorSlotName = "Armor";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString MainSlotName = "Main";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString SecondarySlotName = "Secondary";
	
	UPROPERTY(EditDefaultsOnly, Category = "Character|Equipment")
	FString DetectorSlotName = "Detector";
	
public:
	void OnArmorSlotChanged(class UItemObject* ItemObject);
	void OnMainSlotChanged(UItemObject* ItemObject);
	void OnSecondarySlotChanged(UItemObject* ItemObject);
	void OnDetectorSlotChanged(UItemObject* ItemObject);
};
