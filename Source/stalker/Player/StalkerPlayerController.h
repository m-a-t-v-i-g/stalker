// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "InventoryManagerComponent.h"
#include "StalkerPlayerController.generated.h"

class UOrganicAbilityComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UCharacterArmorComponent;
class UInventoryManagerComponent;
class UPawnInteractionComponent;
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
	Trading,
	Upgrading
};

USTRUCT()
struct FPlayerInitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	APlayerCharacter* Character = nullptr;
	
	UPROPERTY()
	UOrganicAbilityComponent* AbilitySystemComponent = nullptr;
	
	UPROPERTY()
	UInventoryComponent* InventoryComponent = nullptr;
	
	UPROPERTY()
	UEquipmentComponent* EquipmentComponent = nullptr;
	
	UPROPERTY()
	UCharacterArmorComponent* ArmorComponent = nullptr;
	
	UPROPERTY()
	UInventoryManagerComponent* InventoryManager = nullptr;
	
	UPROPERTY()
	UPawnInteractionComponent* InteractionComponent = nullptr;

	FPlayerInitInfo()
	{
	}

	FPlayerInitInfo(APlayerCharacter* Char, UOrganicAbilityComponent* AbilityComp,
	                UInventoryComponent* InventoryComp, UEquipmentComponent* EquipmentComp,
	                UPawnInteractionComponent* InteractionComp,
	                UInventoryManagerComponent* InventoryMgr) : Character(Char),
	                                                            AbilitySystemComponent(AbilityComp),
	                                                            InventoryComponent(InventoryComp),
	                                                            EquipmentComponent(EquipmentComp),
	                                                            InventoryManager(InventoryMgr),
	                                                            InteractionComponent(InteractionComp)
	{
	}

	void AddArmorComponent(UCharacterArmorComponent* ArmorComp)
	{
		ArmorComponent = ArmorComp;
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
	FORCEINLINE UInventoryManagerComponent* GetInventoryManager() const { return InventoryManager; }

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
	TObjectPtr<UInventoryManagerComponent> InventoryManager;
	
	bool bIsPawnInitialized = false;
};
