// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "ItemDraggedWidget.generated.h"

UCLASS()
class STALKER_API UItemDraggedWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Interactive Item")
	TSoftObjectPtr<UDataTable> DescriptionsTable;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemImage;

public:
	void SetupDraggedWidget(const FGameplayTag& ItemTag, FIntPoint Size);
};
