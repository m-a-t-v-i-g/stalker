// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterArtifactComponent.generated.h"

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterArtifactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterArtifactComponent();
};
