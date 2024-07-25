// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "InteractiveItemWidget.generated.h"

class UItemObject;

DECLARE_DELEGATE_OneParam(FOnInteractiveItemOperationSignature, UItemObject*);

UCLASS()
class STALKER_API UInteractiveItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;
	
	UPROPERTY(EditAnywhere, Category = "Interactive Item")
	TSubclassOf<class UItemDraggedWidget> ItemDraggedWidgetClass;
	
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
	TWeakObjectPtr<UItemObject> BoundObject;

public:
	FOnInteractiveItemOperationSignature OnBeginDragDropOperation;
	FOnInteractiveItemOperationSignature OnCompleteDragDropOperation;
	FOnInteractiveItemOperationSignature OnReverseDragDropOperation;

	virtual void InitItemWidget(const FGameplayTag& InItemTag, UItemObject* BindObject, FIntPoint Size);

protected:
	virtual FReply HandleLeftMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey);
	virtual FReply HandleRightMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey);

public:
	void BeginDragOperation();
	void CompleteDragOperation();
	void ReverseDragOperation();
	
	const UItemObject* GetBoundObject() const { return BoundObject.Get(); }

	template <class T>
	const T* GetBoundObject() const { return Cast<T>(GetBoundObject()); }
	
protected:
	UFUNCTION()
	ESlateVisibility GetAmountVisibility();
	
	UFUNCTION()
	FText GetAmountText();
};
