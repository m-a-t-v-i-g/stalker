// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSystemCore.h"
#include "Components/ActorComponent.h"
#include "CharacterOutfitComponent.generated.h"

class UAbilitySystemComponent;
struct FEquipmentSlotChangeData;
class UInventoryComponent;
class UEquipmentComponent;
class UCharacterStateComponent;
class UHitScanComponent;
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

	void OnEquipmentSlotChanged(const FEquipmentSlotChangeData& SlotData, FString SlotName);
	virtual void OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
	                                const FHitResult& HitResult, float DamageValue);
	virtual void OnCharacterDead();
	
	virtual void ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName);

	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;

	FORCEINLINE FOutfitSlot* FindOutfitSlot(const FString& SlotName);

	AStalkerCharacter* GetCharacter() const { return CharacterRef; }

	UAbilitySystemComponent* GetAbilityComponent() const { return AbilityComponentRef; }
	UInventoryComponent* GetCharacterInventory() const { return InventoryComponentRef; }
	UEquipmentComponent* GetCharacterEquipment() const { return EquipmentComponentRef; }
	UCharacterStateComponent* GetStateComponent() const { return StateComponentRef; }
	UHitScanComponent* GetHitScanComponent() const { return HitScanComponentRef; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Outfit")
	TObjectPtr<const UItemBehaviorConfig> ItemBehaviorConfig;

	UPROPERTY(EditAnywhere, Category = "Outfit")
	TArray<FOutfitSlot> OutfitSlots;
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem);
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem);
	
private:
	TObjectPtr<AStalkerCharacter> CharacterRef;

	TObjectPtr<UAbilitySystemComponent> AbilityComponentRef;
	TObjectPtr<UInventoryComponent> InventoryComponentRef;
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	TObjectPtr<UCharacterStateComponent> StateComponentRef;
	TObjectPtr<UHitScanComponent> HitScanComponentRef;
};
