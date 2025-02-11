// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemSystemCore.h"
#include "Components/PawnComponent.h"
#include "CharacterOutfitComponent.generated.h"

struct FEquipmentSlotChangeData;
class UAbilitySystemComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UCharacterStateComponent;
class UHitScanComponent;
class UItemObject;
class UItemBehaviorSet;
class AStalkerCharacter;

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterOutfitComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UCharacterOutfitComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void SetupOutfitComponent(AStalkerCharacter* InCharacter);

	void OnEquipmentSlotChange(const FEquipmentSlotChangeData& SlotData);
	
	virtual void OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
	                                const FHitResult& HitResult, float DamageValue);
	virtual void OnCharacterDead();
	
	virtual UItemObject* ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName, UItemObject* ItemObject);

	FOutfitSlot* FindOutfitSlot(const FString& SlotName);

	AStalkerCharacter* GetCharacter() const { return CharacterRef; }

	UAbilitySystemComponent* GetAbilityComponent() const { return AbilityComponentRef; }
	UInventoryComponent* GetCharacterInventory() const { return InventoryComponentRef; }
	UEquipmentComponent* GetCharacterEquipment() const { return EquipmentComponentRef; }
	UCharacterStateComponent* GetStateComponent() const { return StateComponentRef; }
	UHitScanComponent* GetHitScanComponent() const { return HitScanComponentRef; }
	
protected:
	virtual void InitializeComponent() override; 
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* InItem);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnEquipSlot", Category = "Outfit")
	void K2_OnEquipSlot(const FString& SlotName, UItemObject* InItem);
	
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnUnequipSlot", Category = "Outfit")
	void K2_OnUnequipSlot(const FString& SlotName, UItemObject* InItem);

	FORCEINLINE const UItemBehaviorSet* GetItemBehaviorSet() const;

private:
	UPROPERTY(Replicated)
	FOutfitList OutfitList;
	
	UPROPERTY()
	TObjectPtr<AStalkerCharacter> CharacterRef;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilityComponentRef;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponentRef;

	UPROPERTY()
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponentRef;

	UPROPERTY()
	TObjectPtr<UHitScanComponent> HitScanComponentRef;

	UPROPERTY()
	TSoftObjectPtr<const UItemBehaviorSet> ItemBehavior;
};
