// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

UENUM()
enum class EItemDragDropOperationResult : uint8
{
	Inventory,
	Equipment,
	FastSlot,
	Drop
};

UCLASS()
class STALKER_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	template <class T>
	T* GetPayload() const
	{
		return Cast<T>(Payload);
	}
};
