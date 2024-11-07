// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "Character/StalkerCharacter.h"
#include "Player/StalkerPlayerController.h"
#include "PlayerCharacter.generated.h"

class UAbilitySet;
class UPlayerInputConfig;
class UInteractionComponent;

UCLASS()
class STALKER_API APlayerCharacter : public AStalkerCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	static FName InteractionComponentName;
	
	virtual void SetupCharacterLocally(AController* NewController) override;

	void OnHUDTabChanged(EHUDTab Tab);

	UFUNCTION(BlueprintCallable, Category = "Character")
	FORCEINLINE UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	template <class T>
	T* GetInteractionComponent() const
	{
		return Cast<T>(GetInteractionComponent());
	}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UPlayerInputConfig> InputConfig;

	virtual void PostInitializeComponents() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void BindDirectionalInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindViewInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindKeyInput(UInputComponent* PlayerInputComponent) override;

	void IA_Move(const FInputActionValue& Value);
	void IA_View(const FInputActionValue& Value);
	
	void IA_Slot(const FInputActionValue& Value);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInteractionComponent> InteractionComponent;
};
