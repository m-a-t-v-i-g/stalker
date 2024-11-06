// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PickUpComponent.generated.h"

class UInteractionComponent;
class UInventoryComponent;
class AItemActor;

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UPickUpComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPickUpComponent();

	void SetupPickUpComponent(UInteractionComponent* InteractionComp, UInventoryComponent* InventoryComp);

	bool TryPickUpItem(AItemActor* ItemActor);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Pick Up Component")
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	UPROPERTY(EditAnywhere, Category = "Pick Up Component")
	TObjectPtr<UInventoryComponent> TargetInventory;
};
