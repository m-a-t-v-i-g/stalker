// Fill out your copyright notice in the Description page of Project Settings.

#include "CircumferenceMarkerWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CircumferenceMarkerWidget)

class SWidget;

UCircumferenceMarkerWidget::UCircumferenceMarkerWidget(const FObjectInitializer& ObjectInitializer)
{
	UWidget::SetVisibility(ESlateVisibility::HitTestInvisible);
	bIsVolatile = true;
}

void UCircumferenceMarkerWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyMarkerWidget->SetRadius(Radius);
	MyMarkerWidget->SetMarkerList(MarkerList);
}

void UCircumferenceMarkerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	
	MyMarkerWidget.Reset();
}

TSharedRef<SWidget> UCircumferenceMarkerWidget::RebuildWidget()
{
	MyMarkerWidget = SNew(SCircumferenceMarkerSlate)
		.MarkerBrush(&MarkerImage)
		.Radius(Radius)
		.MarkerList(MarkerList);

	return MyMarkerWidget.ToSharedRef();
}

void UCircumferenceMarkerWidget::SetRadius(float InRadius)
{
	Radius = InRadius;
	if (MyMarkerWidget.IsValid())
	{
		MyMarkerWidget->SetRadius(InRadius);
	}
}
