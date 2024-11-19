// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "PlayerInventoryManagerComponent.h"
#include "StalkerPlayerController.generated.h"

class APlayerCharacter;
class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UPlayerInventoryManagerComponent;
class UPawnInteractionComponent;

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
struct FPlayerCharacterInitInfo
{
	GENERATED_USTRUCT_BODY()

	APlayerCharacter* Character = nullptr;
	UOrganicAbilityComponent* AbilitySystemComponent = nullptr;
	UCharacterInventoryComponent* InventoryComponent = nullptr;
	UPlayerInventoryManagerComponent* InventoryManager = nullptr;
	UPawnInteractionComponent* InteractionComponent = nullptr;

	FPlayerCharacterInitInfo() {}

	FPlayerCharacterInitInfo(APlayerCharacter* Char, UOrganicAbilityComponent* AbilityComp,
	                         UCharacterInventoryComponent* InventoryComp,
	                         UPlayerInventoryManagerComponent* InventoryMgr,
	                         UPawnInteractionComponent* InteractionComp) : Character(Char),
	                                                                       AbilitySystemComponent(AbilityComp),
	                                                                       InventoryComponent(InventoryComp),
	                                                                       InventoryManager(InventoryMgr),
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
