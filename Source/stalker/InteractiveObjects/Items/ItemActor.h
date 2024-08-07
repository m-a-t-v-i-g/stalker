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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	static FName SphereComponentName;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PreviewMesh;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(EditInstanceOnly, Replicated, Category = "Item")
	TObjectPtr<UItemObject> ItemObject;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	bool bHidePreviewMeshWhenHanded = false;

private:
	bool bHanded = false;
	
public:
	virtual void InitItem(UItemObject* NewItemObject);

	void SetHandedMode();
	void SetFreeMode();
	
protected:
	void UpdateItem();
	
	template <class T>
	T* GetItemObject() const
	{
		return Cast<T>(ItemObject);
	}
};
