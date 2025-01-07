// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterArtifactComponent.h"

UCharacterArtifactComponent::UCharacterArtifactComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}
