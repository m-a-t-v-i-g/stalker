// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "CircumferenceMarkerSlate.generated.h"

USTRUCT(BlueprintType)
struct FCircumferenceMarkerEntry
{
	GENERATED_BODY()

	// The angle to place this marker around the circle (in degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits=deg))
	float PositionAngle = 0.0f;

	// The angle to rotate the marker image (in degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits=deg))
	float ImageRotationAngle = 0.0f;
};

class STALKER_API SCircumferenceMarkerSlate : public SLeafWidget
{
	SLATE_BEGIN_ARGS(SCircumferenceMarkerSlate)
	: _MarkerBrush(FCoreStyle::Get().GetBrush("Throbber.CircleChunk"))
	, _Radius(48.0f)
	{
	}
	SLATE_ARGUMENT(const FSlateBrush*, MarkerBrush)
	SLATE_ARGUMENT(TArray<FCircumferenceMarkerEntry>, MarkerList)
	SLATE_ATTRIBUTE(float, Radius)
	SLATE_ATTRIBUTE(FSlateColor, ColorAndOpacity)
	SLATE_END_ARGS()

	SCircumferenceMarkerSlate();
	
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	                      bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual bool ComputeVolatility() const override { return true; }

	void SetRadius(float NewRadius);
	void SetMarkerList(TArray<FCircumferenceMarkerEntry>& NewMarkerList);

private:
	const FSlateBrush* MarkerBrush = nullptr;

	TArray<FCircumferenceMarkerEntry> MarkerList;

	TAttribute<float> Radius;

	TAttribute<FSlateColor> ColorAndOpacity;
	bool bColorAndOpacitySet = false;

	uint8 bReticleCornerOutsideSpreadRadius : 1;
	
	FSlateRenderTransform GetMarkerRenderTransform(const FCircumferenceMarkerEntry& Marker, const float BaseRadius,
	                                               const float HUDScale) const;
};
