// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "StalkerPlayerController.h"
#include "Characters/StalkerCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilitySet;
class UPlayerInputConfig;

UCLASS()
class STALKER_API APlayerCharacter : public AStalkerCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void PostInitializeComponents() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void BindDirectionalInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindViewInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindKeyInput(UInputComponent* PlayerInputComponent) override;

	virtual bool CheckReloadAbility() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputDataAsset> GeneralInputData;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputLeftMouseName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputRightMouseName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputMoveName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputViewName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputJumpName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputCrouchName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputSprintName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputSlotName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputReloadName;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputAbilityName;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UPlayerInputConfig* InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TObjectPtr<UAbilitySet> AbilitySet;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystem")
	TObjectPtr<UPlayerInputConfig> PlayerAbilitiesDataAsset;

protected:
	void SetCharacterData();
	
	void IA_LeftMouseButton(const FInputActionValue& Value);
	void IA_RightMouseButton(const FInputActionValue& Value);
	
	void IA_Move(const FInputActionValue& Value);
	void IA_View(const FInputActionValue& Value);
	
	void IA_Jump(const FInputActionValue& Value);
	void IA_Crouch(const FInputActionValue& Value);
	void IA_Sprint(const FInputActionValue& Value);
	void IA_Slot(const FInputActionValue& Value);
	void IA_Reload(const FInputActionValue& Value);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

public:
	virtual void SetupCharacterLocally(AController* NewController) override;

	void OnHUDTabChanged(EHUDTab Tab);
	
	FORCEINLINE UPlayerInputConfig* GetPlayerGameplayAbilitiesDataAsset() const { return PlayerAbilitiesDataAsset; }
};
