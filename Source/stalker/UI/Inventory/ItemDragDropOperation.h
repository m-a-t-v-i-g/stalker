// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

UCLASS()
class STALKER_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	bool bWasSuccessful = false;
	bool bTryRecoveryItem = false;

	template <class T>
	T* GetPayload() const
	{
		return Cast<T>(Payload);
	}
};
