// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class USphereComponent;
class UItemObject;

UCLASS()
class STALKER_API AItemActor : public AActor
{
	GENERATED_BODY()

public:
	AItemActor();

	static FName SphereComponentName;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
	
	UPROPERTY(EditInstanceOnly, Category = "Item")
	TObjectPtr<UItemObject> ItemObject;
	
public:
	virtual void InitItem(UItemObject* NewItemObject);

protected:
	template <class T>
	T* GetItemObject() const
	{
		return Cast<T>(ItemObject);
	}
};
