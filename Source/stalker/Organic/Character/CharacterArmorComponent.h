// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSystemCore.h"
#include "Components/ActorComponent.h"
#include "Data/ArmorBehaviorConfig.h"
#include "CharacterArmorComponent.generated.h"

struct FUpdatedSlotData;
class UItemObject;
class UCharacterStateComponent;
class UCharacterInventoryComponent;
class UArmorBehaviorConfig;
class AStalkerCharacter;

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterArmorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterArmorComponent();

	virtual void SetupArmorComponent(AStalkerCharacter* InCharacter);
	virtual void InitCharacterInfo(AController* InController);

	void OnSlotEquipped(const FUpdatedSlotData& SlotData, FString SlotName);
	
	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;
	
	FORCEINLINE FOutfitSlot* FindArmorSlot(const FString& SlotName);

	const FArmorBehavior* GetArmorBehavior(const FName& ItemScriptName) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Armor")
	TObjectPtr<const UArmorBehaviorConfig> ArmorBehaviorConfig;

	UPROPERTY(EditAnywhere, Category = "Armor")
	TArray<FOutfitSlot> ArmorSlots;
	
	void EquipSlot(const FString& SlotName, UItemObject* IncomingItem);
	void UnequipSlot(const FString& SlotName);
	
private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;
	
	TObjectPtr<UCharacterInventoryComponent> InventoryComponentRef;
	TObjectPtr<UCharacterStateComponent> StateComponentRef;
};
