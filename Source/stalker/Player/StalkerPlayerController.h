// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "PlayerInventoryManagerComponent.h"
#include "StalkerPlayerController.generated.h"

class UOrganicAbilityComponent;
class UCharacterInventoryComponent;
class UPawnInteractionComponent;
class UPlayerInventoryManagerComponent;
class UPlayerInputConfig;
class APlayerCharacter;

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
struct FPlayerInitInfo
{
	GENERATED_USTRUCT_BODY()

	APlayerCharacter* Character = nullptr;
	UOrganicAbilityComponent* AbilitySystemComponent = nullptr;
	UCharacterInventoryComponent* InventoryComponent = nullptr;
	UPawnInteractionComponent* InteractionComponent = nullptr;
	UPlayerInventoryManagerComponent* InventoryManager = nullptr;

	FPlayerInitInfo() {}

	FPlayerInitInfo(APlayerCharacter* Char, UOrganicAbilityComponent* AbilityComp,
	                         UCharacterInventoryComponent* InventoryComp,
	                         UPawnInteractionComponent* InteractionComp,
							 UPlayerInventoryManagerComponent* InventoryMgr) : Character(Char),
	                                                                       AbilitySystemComponent(AbilityComp),
	                                                                       InventoryComponent(InventoryComp),
	                                                                       InteractionComponent(InteractionComp),
	                                                                       InventoryManager(InventoryMgr)
	{
	}
};

UCLASS()
class STALKER_API AStalkerPlayerController : public AGenPlayerController
{
	GENERATED_BODY()

public:
	AStalkerPlayerController();
	
	static FName InventoryManagerComponentName;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	
	virtual void SetupInputComponent() override;

	virtual void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	UFUNCTION(BlueprintCallable, Category = "Character")
	FORCEINLINE UPlayerInventoryManagerComponent* GetInventoryManager() const { return InventoryManager; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	TObjectPtr<class AStalkerHUD> StalkerHUD;
	
	TObjectPtr<APlayerCharacter> Stalker;

	void SetupPawn();
	void ConnectHUD();

	void InitEssentialComponents();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerInventoryManagerComponent> InventoryManager;
	
	bool bIsPawnInitialized = false;
};
