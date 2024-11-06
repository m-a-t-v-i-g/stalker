// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class USphereComponent;
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
	
	void BindItemObject(UItemObject* NewItemObject);
	virtual void OnBindItem();

	void UnbindItemObject();
	virtual void OnUnbindItem();
	
	void SetEquipped();
	void SetGrounded();

	virtual void UpdateItem();
	
	UItemObject* GetItemObject() const { return ItemObject; }
	
protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> PhysicCollision;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> InteractionSphere;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = "OnRep_ItemObject", Category = "Item")
	TObjectPtr<UItemObject> ItemObject;
	
	UFUNCTION()
	void OnRep_ItemObject();
	
	UFUNCTION()
	void OnRep_Handed();
	
	template <class T>
	T* GetItemObject() const
	{
		return Cast<T>(ItemObject);
	}
	
private:
	UPROPERTY(ReplicatedUsing = "OnRep_Handed")
	bool bHanded = false;
};
