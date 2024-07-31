// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "StalkerPlayerController.generated.h"

UCLASS()
class STALKER_API AStalkerPlayerController : public AGenPlayerController
{
	GENERATED_BODY()

public:
	AStalkerPlayerController();

	virtual void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputDataAsset> InputData;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputInventoryName;

	UPROPERTY(EditDefaultsOnly, DisplayName = "Input PDA Name", Category = "Input")
	FString InputPDAName;

	TObjectPtr<class APlayerHUD> StalkerHUD;
	
	TObjectPtr<class APlayerCharacter> Stalker;

private:
	bool bIsStalkerInitialized = false;
	
public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	
	virtual void SetupInputComponent() override;

protected:
	void IA_Inventory(const FInputActionValue& Value);
	void IA_PDA(const FInputActionValue& Value);
};
