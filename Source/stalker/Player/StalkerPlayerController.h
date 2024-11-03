// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "StalkerPlayerController.generated.h"

UENUM()
enum class EHUDTab : uint8
{
	HUD,
	Inventory
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHUDTabChanged, EHUDTab);

UCLASS()
class STALKER_API AStalkerPlayerController : public AGenPlayerController
{
	GENERATED_BODY()

public:
	AStalkerPlayerController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	
	virtual void SetupInputComponent() override;

	virtual void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputDataAsset> GeneralInputData;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FString InputInventoryName;

	UPROPERTY(EditDefaultsOnly, DisplayName = "Input PDA Name", Category = "Input")
	FString InputPDAName;

	TObjectPtr<class AStalkerHUD> StalkerHUD;
	
	TObjectPtr<class APlayerCharacter> Stalker;

public:
	FOnHUDTabChanged OnHUDTabChanged;
	
private:
	bool bIsControllerInitialized = false;
	
protected:
	void IA_Inventory(const FInputActionValue& Value);
	void IA_PDA(const FInputActionValue& Value);
	
	void ToggleHUD(EHUDTab Tab, bool bForce = false) const;
};
