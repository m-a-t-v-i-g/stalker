// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosionArea.generated.h"

class USphereComponent;

UCLASS()
class STALKER_API AExplosionArea : public AActor
{
	GENERATED_BODY()

public:
	AExplosionArea();
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion Area", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> ExplosionZone;
};
