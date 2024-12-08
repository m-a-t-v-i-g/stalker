// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterOutfitComponent.h"
#include "CharacterArtifactComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterArtifactComponent : public UCharacterOutfitComponent
{
	GENERATED_BODY()

public:
	UCharacterArtifactComponent();
};
