// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

class UItemWidget;

UENUM(BlueprintType)
enum class EDragDropOperationResult : uint8
{
	None,
	Failed,
	Completed
};

UCLASS()
class STALKER_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	EDragDropOperationResult DragDropOperationResult = EDragDropOperationResult::Failed;

	template <class T>
	T* GetPayload() const
	{
		return Cast<T>(Payload);
	}
};
