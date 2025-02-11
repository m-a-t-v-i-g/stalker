// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OutfitComponent.h"
#include "CharacterOutfitComponent.generated.h"

class UAbilitySystemComponent;
class UInventoryComponent;
class UCharacterStateComponent;
class UHitScanComponent;
class UItemBehaviorSet;
class AStalkerCharacter;

UCLASS()
class STALKER_API UCharacterOutfitComponent : public UOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterOutfitComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	AStalkerCharacter* GetCharacter() const { return CharacterRef; }

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	UAbilitySystemComponent* GetAbilityComponent() const { return AbilityComponentRef; }

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	UInventoryComponent* GetCharacterInventory() const { return InventoryComponentRef; }

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	UCharacterStateComponent* GetStateComponent() const { return StateComponentRef; }

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	UHitScanComponent* GetHitScanComponent() const { return HitScanComponentRef; }

protected:
	virtual void InitializeComponent() override;
	
	virtual void SetupOutfitComponent() override;

	virtual void OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
									const FHitResult& HitResult, float DamageValue);
	virtual void OnCharacterDead();

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	const UItemBehaviorSet* GetItemBehaviorSet() const;

private:
	UPROPERTY()
	TObjectPtr<AStalkerCharacter> CharacterRef;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilityComponentRef;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponentRef;

	UPROPERTY()
	TObjectPtr<UHitScanComponent> HitScanComponentRef;

	UPROPERTY()
	TObjectPtr<UCharacterStateComponent> StateComponentRef;

	UPROPERTY()
	TSoftObjectPtr<const UItemBehaviorSet> ItemBehavior;
};
