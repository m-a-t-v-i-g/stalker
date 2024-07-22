// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemsContainerWidget.h"
#include "ItemsContainerGridWidget.h"

void UItemsContainerWidget::InitializeContainer(UItemsContainerComponent* OwnContainerComp)
{
	ItemsContainerGrid->SetupContainerGrid(OwnContainerComp);
}
