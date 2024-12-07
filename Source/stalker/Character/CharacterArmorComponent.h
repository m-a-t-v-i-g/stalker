// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterOutfitComponent.h"
#include "ItemSystemCore.h"
#include "Data/ItemBehaviorConfig.h"
#include "CharacterArmorComponent.generated.h"

struct FUpdatedSlotData;
class UItemObject;
class UCharacterStateComponent;
class UCharacterInventoryComponent;
class AStalkerCharacter;

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterArmorComponent : public UCharacterOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterArmorComponent();

	const FArmorBehavior* GetArmorBehavior(const FName& ItemScriptName) const;
	
protected:
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem) override;
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem) override;
};
