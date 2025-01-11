// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CircumferenceMarkerSlate.h"
#include "Components/Widget.h"
#include "CircumferenceMarkerWidget.generated.h"

UCLASS()
class STALKER_API UCircumferenceMarkerWidget : public UWidget
{
	GENERATED_BODY()

public:
	UCircumferenceMarkerWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	TArray<FCircumferenceMarkerEntry> MarkerList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance, meta = (ClampMin = 0.0))
	float Radius = 48.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	FSlateBrush MarkerImage;

	/** Whether reticle corner images are placed outside the spread radius */
	//@TODO: Make this a 0-1 float alignment instead (e.g., inside/on/outside the radius)?
	UPROPERTY(EditAnywhere, Category = Corner)
	uint8 bReticleCornerOutsideSpreadRadius : 1;

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	void SetRadius(float InRadius);
	
private:
	/** Internal slate widget representing the actual marker visuals */
	TSharedPtr<SCircumferenceMarkerSlate> MyMarkerWidget;
};
