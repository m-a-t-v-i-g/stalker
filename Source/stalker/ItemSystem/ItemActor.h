// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UItemDefinition;
class UItemPredictedData;
class UItemObject;

UCLASS()
class STALKER_API AItemActor : public AInteractiveActor
{
	GENERATED_BODY()

	friend UItemObject;

public:
	AItemActor(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Destroyed() override;

	virtual bool OnInteract(AActor* Interactor) override;
	
	void BindItemObject(UItemObject* NewItemObject);
	virtual void OnBindItem();

	void UnbindItemObject();
	virtual void OnUnbindItem(UItemObject* PrevItemObject);
	
	virtual void UpdateItem();
	
	virtual void SetGrounded();
	virtual void SetCollected();
	virtual void SetEquipped();

	bool IsBoundItem() const { return ItemObject != nullptr; }

	USkeletalMeshComponent* GetMesh() const { return Mesh; } 
	
	UItemObject* GetItemObject() const { return ItemObject; }
	
	template <class T>
	T* GetItemObject() const
	{
		return Cast<T>(ItemObject);
	}
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Starting Data")
	bool bUsePredictedData = false;
	
	UPROPERTY(EditInstanceOnly, Instanced, Category = "Starting Data", meta = (EditCondition = "bUsePredictedData"))
	TObjectPtr<UItemPredictedData> PredictedData;
	
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TObjectPtr<const UItemDefinition> ItemDefinition;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_ItemObject(UItemObject* PrevItemObject);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_ItemObject", Category = "Item",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UItemObject> ItemObject;
};
