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

	TMulticastDelegate<void(float)> OnTotalArmorEnduranceChangedDelegate;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnCharacterDamaged(const FGameplayTag& DamageTag, const FGameplayTag& PartTag,
	                                const FHitResult& HitResult, float DamageValue) override;
	
	bool EquipArmor(UItemObject* ItemObject, FEquippedArmorData& ArmorData);
	void UnequipArmor(UItemObject* ItemObject, FEquippedArmorData& ArmorData);

	void OnEquippedArmorEnduranceChanged(float ItemEndurance, UItemObject* ItemObject);

	float CalculateTotalArmorEndurance();
	
	const FArmorBehavior* GetArmorBehavior(const FName& ItemScriptName) const;

	int GetArmorPartsNum() const { return EquippedArmorParts.Num(); }
	float GetTotalArmorEndurance() const { return TotalArmorEndurance; }
	
protected:
	virtual void OnEquipSlot(const FString& SlotName, UItemObject* IncomingItem) override;
	virtual void OnUnequipSlot(const FString& SlotName, UItemObject* OutgoingItem) override;

	FActiveGameplayEffectHandle ApplyItemEffectSpec(UItemObject* ItemObject);
	bool RemoveItemEffectSpec(UItemObject* ItemObject);
	void ReapplyItemEffectSpec(UItemObject* ItemObject);
	
	USkeletalMeshComponent* GetCharacterMesh() const;

	UFUNCTION()
	void OnRep_TotalArmorEndurance();
	
private:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Armor")
	FEquippedArmorData EquippedHelmetData;

	UPROPERTY(EditInstanceOnly, Replicated, Category = "Armor")
	FEquippedArmorData EquippedBodyData;

	UPROPERTY(EditInstanceOnly, Category = "Armor")
	TMap<FGameplayTag, UItemObject*> EquippedArmorParts;
	
	UPROPERTY(EditInstanceOnly, Category = "Armor")
	TMap<UItemObject*, FActiveGameplayEffectHandle> ActiveItemEffects;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_TotalArmorEndurance", Category = "Armor", meta = (ForceUnits = "%"))
	float TotalArmorEndurance = 0.0f;
};
