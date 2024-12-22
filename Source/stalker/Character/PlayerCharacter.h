// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "StalkerCharacter.h"
#include "StalkerPlayerController.h"
#include "PlayerCharacter.generated.h"

class UAbilitySet;
class UPlayerInputConfig;
class UPawnInteractionComponent;

UCLASS()
class STALKER_API APlayerCharacter : public AStalkerCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	static FName InteractionComponentName;

	TMulticastDelegate<void()> OnToggleInventory;
	TMulticastDelegate<void(float)> OnToggleSlot;

	virtual void InitCharacterComponents() override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	FORCEINLINE UPawnInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UPlayerInputConfig> InputConfig;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BindDirectionalInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindViewInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindKeyInput(UInputComponent* PlayerInputComponent) override;

	void IA_Move(const FInputActionValue& Value);
	void IA_View(const FInputActionValue& Value);

	void IA_Inventory(const FInputActionValue& Value);
	void IA_Slot(const FInputActionInstance& InputAction);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPawnInteractionComponent> InteractionComponent;
};
