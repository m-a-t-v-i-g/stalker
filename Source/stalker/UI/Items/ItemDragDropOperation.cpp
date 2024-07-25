// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemDragDropOperation.h"
#include "InteractiveItemWidget.h"

void UItemDragDropOperation::CompleteDragDropOperation()
{
	ItemWidgetRef->CompleteDragOperation();
}

void UItemDragDropOperation::ReverseDragDropOperation()
{
	ItemWidgetRef->ReverseDragOperation();
}
