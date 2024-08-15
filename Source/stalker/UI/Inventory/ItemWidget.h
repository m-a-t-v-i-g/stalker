// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Library/Items/ItemsLibrary.h"
#include "ItemWidget.generated.h"

class UItemObject;

DECLARE_DELEGATE_OneParam(FDragDropOperationSignature, UItemObject*);
DECLARE_DELEGATE_TwoParams(FDragDropOperationResultSignature, UItemObject*, EDragDropOperationResult);

UCLASS()
class STALKER_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;

	UPROPERTY(EditAnywhere, Category = "Item")
	float DragOpacity = 0.77f;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> ItemImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextAmount;

private:
	TWeakObjectPtr<UItemObject> BoundObject;

public:
	FDragDropOperationSignature OnMouseEnter;
	FDragDropOperationSignature OnMouseLeave;
	FDragDropOperationSignature OnDoubleClick
	;
	FDragDropOperationSignature OnBeginDragDropOperation;
	FDragDropOperationSignature OnReverseDragDropOperation;
	FDragDropOperationResultSignature OnCompleteDragDropOperation;

	virtual void InitItemWidget(UItemObject* BindObject, FIntPoint Size);

protected:
	virtual FReply HandleLeftMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey);
	virtual FReply HandleLeftMouseButtonDownDoubleClick(const FPointerEvent& InMouseEvent, const FKey& DragKey);
	
	virtual FReply HandleRightMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey);

public:
	void RotateItem();
	void UnRotateItem();

	void MouseEnter();
	void MouseLeave();
	void DoubleClick();

	void BeginDragOperation();
	void ReverseDragOperation();
	void CompleteDragOperation(EDragDropOperationResult OperationResult);
	
	const UItemObject* GetBoundObject() const { return BoundObject.Get(); }

	template <class T>
	const T* GetBoundObject() const { return Cast<T>(GetBoundObject()); }
	
protected:
	UFUNCTION()
	ESlateVisibility GetAmountVisibility();
	
	UFUNCTION()
	FText GetAmountText();
};
