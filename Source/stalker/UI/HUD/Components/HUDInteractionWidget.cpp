// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDInteractionWidget.h"
#include "PawnInteractionComponent.h"
#include "Components/TextBlock.h"

void UHUDInteractionWidget::SetupInteractionWidget(UPawnInteractionComponent* InteractionComp)
{
	OwnInteractionComponent = InteractionComp;
	if (OwnInteractionComponent.IsValid())
	{
		OnDetectedActorChanged(OwnInteractionComponent->GetDetectedActor());
		OwnInteractionComponent->OnDetectedActorChanged.AddUObject(this, &UHUDInteractionWidget::OnDetectedActorChanged);
	}
}

void UHUDInteractionWidget::ClearInteractionWidget()
{
	if (OwnInteractionComponent.IsValid())
	{
		OnDetectedActorChanged(nullptr);
		OwnInteractionComponent->OnDetectedActorChanged.RemoveAll(this);
	}
	
	OwnInteractionComponent.Reset();
}

void UHUDInteractionWidget::OnDetectedActorChanged(const AActor* DetectedActor)
{
	DetectedActor != nullptr
		? InteractionText->SetVisibility(ESlateVisibility::HitTestInvisible)
		: InteractionText->SetVisibility(ESlateVisibility::Collapsed);
}
