// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Library/Items/ItemsLibrary.h"
#include "ItemDragDropOperation.generated.h"

class UItemWidget;

UCLASS()
class STALKER_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UItemWidget* ItemWidgetRef;

	UUserWidget* SourceWidget = nullptr;
	
	bool bWasSuccessful = false;
	
	void NotifySourceAboutDropOperation(EDragDropOperationResult OperationResult) const;
	
	template <class T>
	T* GetPayload() const
	{
		return Cast<T>(Payload);
	}
};
