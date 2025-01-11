// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/StalkerGameInstance.h"
#include "GameData.h"

UStalkerGameInstance::UStalkerGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UGameData> FoundGameData(TEXT("/Game/stalker/Core/DA_GameData.DA_GameData"));
	GameData = FoundGameData.Object;
}

const UGameData* UStalkerGameInstance::GetGameData() const
{
	return GameData;
}
