// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractiveItemWidget.generated.h"

UCLASS()
class STALKER_API UInteractiveItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;
	
public:
	virtual void InitItemWidget(const UObject* BindObject, FIntPoint Size);

private:
	TWeakObjectPtr<const UObject> BoundObject;
};
