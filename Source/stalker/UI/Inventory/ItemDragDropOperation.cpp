// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemDragDropOperation.h"
#include "ItemWidget.h"

void UItemDragDropOperation::CompleteDragDropOperation(EDragDropOperationResult OperationResult) const
{
	ItemWidgetRef->CompleteDragOperation(OperationResult);
}

void UItemDragDropOperation::ReverseDragDropOperation() const
{
	ItemWidgetRef->ReverseDragOperation();
}
