// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenGameStateBase.h"
#include "StalkerGameState.generated.h"

class UItemObject;

UCLASS()
class STALKER_API AStalkerGameState : public AGenGameStateBase
{
	GENERATED_BODY()

public:
	void AddItemObject(uint32 ItemId, UItemObject* ItemObject);
	void RemoveItemObject(uint32 ItemId);

	UItemObject* GetItemObjectById(uint32 ItemId) const;
	
	bool IsItemObjectExist(uint32 ItemId) const;
	
private:
	UPROPERTY(EditInstanceOnly, Category = "World Statistic")
	TMap<uint32, UItemObject*> WorldItems;
};
