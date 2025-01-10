// Fill out your copyright notice in the Description page of Project Settings.

#include "GameData.h"
#include "Game/StalkerGameInstance.h"

const UGameData* UGameData::Get(const UObject* WorldContextObject)
{
	ensure(WorldContextObject);
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UStalkerGameInstance* DustGameInstance = World->GetGameInstance<UStalkerGameInstance>())
		{
			return DustGameInstance->GetGameData();
		}
	}
	return nullptr;
}
