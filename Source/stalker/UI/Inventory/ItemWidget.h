// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"

class UItemObject;

DECLARE_DELEGATE_ThreeParams(FDragDropOperationSignature, const FGeometry&, const FPointerEvent&, UDragDropOperation*);

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
	TWeakObjectPtr<const UObject> OwnerPrivate;
	TWeakObjectPtr<UItemObject> BoundObject;

public:
	TDelegate<void(const FGeometry&, const FPointerEvent&, UItemObject*)> OnMouseEnter;
	TDelegate<void(const FPointerEvent&)> OnMouseLeave;
	TDelegate<void(const FGeometry&, const FPointerEvent&, UItemObject*)> OnDoubleClick;
	TDelegate<void(const FGeometry&, const FPointerEvent&, UDragDropOperation*)> OnDragItem;

	virtual void InitItemWidget(const UObject* Owner, UItemObject* BindObject, FIntPoint Size);
	virtual void ClearItemWidget();

protected:
	virtual FReply HandleLeftMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey);
	virtual FReply HandleLeftMouseButtonDownDoubleClick(const FGeometry& InLocalGeometry,
	                                                    const FPointerEvent& InMouseEvent, const FKey& DragKey);
	virtual FReply HandleRightMouseButtonDown(const FPointerEvent& InMouseEvent, const FKey& DragKey);

public:
	void RotateItem();
	void UnRotateItem();

	void MouseEnter(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent);
	void MouseLeave(const FPointerEvent& InMouseEvent);
	void DoubleClick(const FGeometry& InLocalGeometry, const FPointerEvent& InMouseEvent);

	void BeginDragOperation(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation* InOperation);

	const UItemObject* GetBoundObject() const { return BoundObject.Get(); }

	template <class T>
	const T* GetBoundObject() const { return Cast<T>(GetBoundObject()); }
	
protected:
	UFUNCTION()
	ESlateVisibility GetAmountVisibility();
	
	UFUNCTION()
	FText GetAmountText();
};
