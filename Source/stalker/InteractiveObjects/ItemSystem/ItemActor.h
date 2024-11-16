// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class USphereComponent;
class UItemDefinition;
class UItemPredictedData;
class UItemObject;

UCLASS()
class STALKER_API AItemActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AItemActor();

	static FName PhysicCollisionName;
	
	static FName InteractionSphereName;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Destroyed() override;

	virtual bool OnInteract(AActor* Interactor) override;
	
	void BindItemObject(UItemObject* NewItemObject);
	virtual void OnBindItem();

	void UnbindItemObject();
	virtual void OnUnbindItem(UItemObject* PrevItemObject);
	
	virtual void UpdateItem();
	
	virtual void SetEquipped();
	virtual void SetGrounded();

	bool IsBoundItem() const { return ItemObject != nullptr; }
	
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
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> PhysicCollision;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> InteractionSphere;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_ItemObject", Category = "Item")
	TObjectPtr<UItemObject> ItemObject;
	
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                                     const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void OnRep_ItemObject(UItemObject* PrevItemObject);
	
	UFUNCTION()
	void OnRep_Handed();
	
private:
	UPROPERTY(ReplicatedUsing = "OnRep_Handed")
	bool bHanded = false;
};
