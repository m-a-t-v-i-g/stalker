// Fill out your copyright notice in the Description page of Project Settings.

#include "CircumferenceMarker.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CircumferenceMarker)

class SWidget;

UCircumferenceMarker::UCircumferenceMarker(const FObjectInitializer& ObjectInitializer)
{
	UWidget::SetVisibility(ESlateVisibility::HitTestInvisible);
	bIsVolatile = true;
}

void UCircumferenceMarker::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyMarkerWidget->SetRadius(Radius);
	MyMarkerWidget->SetMarkerList(MarkerList);
}

void UCircumferenceMarker::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	
	MyMarkerWidget.Reset();
}

TSharedRef<SWidget> UCircumferenceMarker::RebuildWidget()
{
	MyMarkerWidget = SNew(SCircumferenceMarkerSlate)
		.MarkerBrush(&MarkerImage)
		.Radius(Radius)
		.MarkerList(MarkerList);

	return MyMarkerWidget.ToSharedRef();
}

void UCircumferenceMarker::SetRadius(float InRadius)
{
	Radius = InRadius;
	if (MyMarkerWidget.IsValid())
	{
		MyMarkerWidget->SetRadius(InRadius);
	}
}
