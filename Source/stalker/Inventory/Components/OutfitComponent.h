// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "ItemSystemCore.h"
#include "Components/PawnComponent.h"
#include "OutfitComponent.generated.h"

struct FGameplayEffectSpec;
struct FEquipmentSlotChangeData;

class UEquipmentComponent;
class UItemObject;

USTRUCT(BlueprintType)
struct FActiveOutfitGE
{
	GENERATED_USTRUCT_BODY()

	friend class UOutfitComponent;
	
	FActiveOutfitGE()
	{
	}

private:
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> Handles;
};

UCLASS(ClassGroup = "Stalker")
class STALKER_API UOutfitComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UOutfitComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Outfit")
	UEquipmentComponent* GetCharacterEquipment() const { return EquipmentComponentRef; }

protected:
	virtual void InitializeComponent() override; 
	
	virtual void SetupOutfitComponent();

	void OnEquipmentSlotChange(const FEquipmentSlotChangeData& SlotData);
	
	virtual UItemObject* ArmSlot(const FString& SlotName, UItemObject* ItemObject);
	virtual void DisarmSlot(const FString& SlotName, UItemObject* ItemObject);
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* InItem);
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* PrevItem);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnEquipSlot", Category = "Outfit")
	void K2_OnEquipSlot(const FString& SlotName, UItemObject* InItem);
	
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnUnequipSlot", Category = "Outfit")
	void K2_OnUnequipSlot(const FString& SlotName, UItemObject* InItem);

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	bool ApplyItemGameplayEffects(UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	bool RemoveItemGameplayEffects(UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	void ReapplyItemGameplayEffects(UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	virtual void ModifyItemEffectSpec(FGameplayEffectSpec& Spec, UItemObject* ItemObject);

	UFUNCTION(BlueprintCallable, Category = "Outfit")
	const TMap<UItemObject*, FActiveOutfitGE>& GetActiveItemEffects() const;
	
private:
	UPROPERTY(Replicated)
	FOutfitList OutfitList;
	
	UPROPERTY()
	TObjectPtr<UEquipmentComponent> EquipmentComponentRef;
	
	UPROPERTY()
	TMap<UItemObject*, FActiveOutfitGE> ActiveItemEffects;
};
