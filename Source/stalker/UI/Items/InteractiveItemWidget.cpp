// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractiveItemWidget.h"
#include "Components/SizeBox.h"
#include "Player/PlayerHUD.h"

void UInteractiveItemWidget::InitItemWidget(const UObject* BindObject, FIntPoint Size)
{
	BoundObject = BindObject;

	FVector2D GridSize = {Size.X * APlayerHUD::TileSize, Size.Y * APlayerHUD::TileSize};

	SizeBox->SetWidthOverride(GridSize.X);
	SizeBox->SetHeightOverride(GridSize.Y);
}
