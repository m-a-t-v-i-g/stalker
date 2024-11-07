// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDInteractionWidget.generated.h"

class UInteractionComponent;
class UTextBlock;

UCLASS()
class STALKER_API UHUDInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupInteractionWidget(UInteractionComponent* InteractionComp);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InteractionText;

	void OnDetectedActorChanged(const AActor* DetectedActor);
	
private:
	TWeakObjectPtr<UInteractionComponent> OwnInteractionComponent;
};
