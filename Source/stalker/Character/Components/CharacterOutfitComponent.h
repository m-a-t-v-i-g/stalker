// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSystemCore.h"
#include "Components/ActorComponent.h"
#include "CharacterOutfitComponent.generated.h"

struct FUpdatedSlotData;
class UInventoryComponent;
class UEquipmentComponent;
class UCharacterStateComponent;
class UItemObject;
class UItemBehaviorConfig;
class AStalkerCharacter;

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterOutfitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterOutfitComponent();

	virtual void SetupOutfitComponent(AStalkerCharacter* InCharacter);
	virtual void InitCharacterInfo(AController* InController);

	void OnEquipmentSlotChanged(const FUpdatedSlotData& SlotData, FString SlotName);
	
	virtual void ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName);

	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;

	FORCEINLINE FOutfitSlot* FindOutfitSlot(const FString& SlotName);

	AStalkerCharacter* GetCharacter() const { return CharacterRef; }
	AController* GetController() const { return ControllerRef; }
	
	UInventoryComponent* GetCharacterInventory() const;
	UEquipmentComponent* GetCharacterEquipment() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Outfit")
	TObjectPtr<const UItemBehaviorConfig> ItemBehaviorConfig;

	UPROPERTY(EditAnywhere, Category = "Outfit")
	TArray<FOutfitSlot> OutfitSlots;
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem);
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem);
	
private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;

	TObjectPtr<UInventoryComponent> InventoryComponentRef;
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	TObjectPtr<UCharacterStateComponent> StateComponentRef;
};
