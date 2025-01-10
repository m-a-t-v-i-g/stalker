// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StalkerGameInstance.generated.h"

class UGameData;

UCLASS()
class STALKER_API UStalkerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UStalkerGameInstance(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE const UGameData* GetGameData() const;
	
private:
	TObjectPtr<const UGameData> GameData;
};
