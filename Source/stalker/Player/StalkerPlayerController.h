// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "StalkerPlayerController.generated.h"

class APlayerCharacter;
class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UInteractionComponent;

UENUM()
enum class EHUDTab : uint8
{
	HUD,
	Inventory
};

UENUM()
enum class EInventoryAction : uint8
{
	None,
	Looting,
	Upgrading
};

USTRUCT()
struct FCharacterInitInfo
{
	GENERATED_USTRUCT_BODY()

	APlayerCharacter* Character = nullptr;
	UOrganicAbilityComponent* AbilitySystemComponent = nullptr;
	UCharacterInventoryComponent* InventoryComponent = nullptr;
	UInteractionComponent* InteractionComponent = nullptr;

	FCharacterInitInfo() {}

	FCharacterInitInfo(APlayerCharacter* Char, UOrganicAbilityComponent* AbilityComp,
	                   UCharacterInventoryComponent* InventoryComp,
	                   UInteractionComponent* InteractionComp) : Character(Char), AbilitySystemComponent(AbilityComp),
	                                                             InventoryComponent(InventoryComp),
	                                                             InteractionComponent(InteractionComp)
	{
	}
};

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
	
	TObjectPtr<class AStalkerHUD> StalkerHUD;
	
	TObjectPtr<APlayerCharacter> Stalker;

	void ConnectHUD();
	
private:
	bool bIsControllerInitialized = false;
};
