// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemDragDropOperation.h"
#include "ItemWidget.h"

void UItemDragDropOperation::NotifySourceAboutDropOperation(EDragDropOperationResult OperationResult) const
{
	ItemWidgetRef->NotifyAboutDragOperation(OperationResult);
	ItemWidgetRef->MarkAsGarbage();
}
