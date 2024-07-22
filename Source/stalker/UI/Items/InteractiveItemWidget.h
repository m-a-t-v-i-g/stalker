// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "InteractiveItemWidget.generated.h"

UCLASS()
class STALKER_API UInteractiveItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextAmount;

	UPROPERTY(EditAnywhere, Category = "Interactive Item")
	TSoftObjectPtr<UDataTable> DescriptionsTable;
	
private:
	FGameplayTag ItemTag;
	
	TWeakObjectPtr<const UObject> BoundObject;
	
public:
	virtual void InitItemWidget(const FGameplayTag& InItemTag, const UObject* BindObject, FIntPoint Size);

	const UObject* GetBoundObject() const { return BoundObject.Get(); }

	template <class T>
	const T* GetBoundObject() const { return Cast<T>(GetBoundObject()); }
	
protected:
	UFUNCTION()
	ESlateVisibility GetAmountVisibility();
	
	UFUNCTION()
	FText GetAmountText();
};
