// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawner.generated.h"

class AItemActor;

UCLASS()
class STALKER_API AItemSpawner : public AActor
{
	GENERATED_BODY()

public:
	AItemSpawner();

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<AItemActor> SpawnActorClass;
	
	virtual void BeginPlay() override;
};
