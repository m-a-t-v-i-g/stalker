// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveActor.h"
#include "ContainerActor.generated.h"

class UInventoryComponent;

UCLASS()
class STALKER_API AContainerActor : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AContainerActor();

	static FName InventoryComponentName;
	
	virtual bool OnInteract(AActor* Interactor) override;

	virtual void PostInitializeComponents() override;

protected:
	UFUNCTION()
	void OnTakeDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser );

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;
};
