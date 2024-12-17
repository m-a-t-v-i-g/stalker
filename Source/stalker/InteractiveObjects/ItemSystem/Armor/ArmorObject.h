// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "ArmorObject.generated.h"

class UGameplayEffect;
class AArmorActor;

USTRUCT()
struct FArmorInstanceData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, NotReplicated, Category = "Armor")
	TMap<FGameplayTag, float> ProtectionModifiers;
};

UCLASS()
class STALKER_API UArmorDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Armor")
	TObjectPtr<UDataTable> ArmorPropertiesDataTable;
};

UCLASS()
class STALKER_API UArmorPredictedData : public UItemPredictedData
{
	GENERATED_BODY()
};

UCLASS()
class STALKER_API UArmorInstance : public UItemInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Armor")
	FArmorInstanceData ArmorData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
								 const UItemPredictedData* PredictedData) override;
	virtual void SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
								 const UItemInstance* Instance) override;
};

UCLASS()
class STALKER_API UArmorObject : public UItemObject
{
	GENERATED_BODY()

public:

#pragma region Replication
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
#pragma endregion Replication

#pragma region Behavior
	
	virtual void Use_Implementation(UObject* Source) override;

	virtual void OnEnduranceUpdated(float NewEndurance, float PrevEndurance) override;
	
	virtual void OnBindItemActor() override;
	virtual void OnUnbindItemActor(AItemActor* PrevItemActor) override;

	virtual bool IsCorrespondsTo(const UItemObject* OtherItemObject) const override;

	void UpdateProtectionModifiers() const;
	
	FORCEINLINE const TMap<FGameplayTag, float>& GetProtectionModifiers() const;
	
#pragma endregion Behavior

	FORCEINLINE const UArmorDefinition* GetArmorDefinition() const;
	FORCEINLINE USkeletalMesh* GetVisual() const;
	FORCEINLINE AArmorActor* GetArmorActor() const;
	FORCEINLINE UArmorInstance* GetArmorInstance() const;
	FORCEINLINE FArmorStaticDataTableRow* GetArmorProperties() const;
};
