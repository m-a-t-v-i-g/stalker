// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "CharacterOutfitComponent.h"
#include "Data/ItemBehaviorConfig.h"
#include "CharacterArmorComponent.generated.h"

class UItemObject;

USTRUCT()
struct FEquippedArmorData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<UItemObject> ItemObject;

	UPROPERTY(VisibleInstanceOnly)
	FArmorBehavior ArmorBehavior;
	
	FEquippedArmorData() {}
	
	FEquippedArmorData(UItemObject* ItemObj, const FArmorBehavior& ArmorBeh) : ItemObject(ItemObj), ArmorBehavior(ArmorBeh)
	{
	}

	void Clear()
	{
		ItemObject = nullptr;
		ArmorBehavior.Clear();
	}

	bool IsValid() const
	{
		return ItemObject != nullptr;
	}
};

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterArmorComponent : public UCharacterOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterArmorComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipArmor(UItemObject* ItemObject, FEquippedArmorData& ArmorData);
	void UnequipArmor(UItemObject* ItemObject, FEquippedArmorData& ArmorData);

	void OnEquippedArmorEnduranceChanged(float ItemEndurance, UItemObject* ItemObject);
	
	const FArmorBehavior* GetArmorBehavior(const FName& ItemScriptName) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Armor")
	TMap<FName, FGameplayTag> HitScanMap;
	
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem) override;
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* OutgoingItem) override;

	void OnCharacterDamaged(const FGameplayTag& DamageTag, const FHitResult& HitResult);
	
	FActiveGameplayEffectHandle ApplyItemEffectSpec(UItemObject* ItemObject);
	bool RemoveItemEffectSpec(UItemObject* ItemObject);
	void ReapplyItemEffectSpec(UItemObject* ItemObject);
	
	USkeletalMeshComponent* GetCharacterMesh() const;
	
private:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Armor")
	FEquippedArmorData EquippedHelmetData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Armor")
	FEquippedArmorData EquippedBodyData;

	UPROPERTY(EditInstanceOnly, Category = "Armor")
	TMap<FGameplayTag, UItemObject*> EquippedArmor;
	
	UPROPERTY(EditInstanceOnly, Category = "Armor")
	TMap<UItemObject*, FActiveGameplayEffectHandle> ItemEffects;
};
