// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPlayerController.h"
#include "InputMappingContext.h"
#include "InventoryManagerComponent.h"
#include "StalkerPlayerController.generated.h"

class UAbilitySystemComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UCharacterWeaponComponent;
class UCharacterArmorComponent;
class UInventoryManagerComponent;
class UPawnInteractionComponent;
class UInputConfig;
class APlayerCharacter;

USTRUCT()
struct FCharacterHUDInitData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	AStalkerCharacter* Character = nullptr;
	
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;
	
	UPROPERTY()
	UInventoryComponent* InventoryComponent = nullptr;
	
	UPROPERTY()
	UEquipmentComponent* EquipmentComponent = nullptr;
	
	UPROPERTY()
	UCharacterWeaponComponent* WeaponComponent = nullptr;
	
	UPROPERTY()
	UCharacterArmorComponent* ArmorComponent = nullptr;
	
	UPROPERTY()
	UPawnInteractionComponent* InteractionComponent = nullptr;

	FCharacterHUDInitData() {}

	FCharacterHUDInitData(AStalkerCharacter* Char);

	void AddAbilitySystemComponent(UAbilitySystemComponent* AbilityComp)
	{
		AbilitySystemComponent = AbilityComp;
	}

	void AddInventoryComponent(UInventoryComponent* InventoryComp)
	{
		InventoryComponent = InventoryComp;
	}

	void AddEquipmentComponent(UEquipmentComponent* EquipmentComp)
	{
		EquipmentComponent = EquipmentComp;
	}

	void AddWeaponComponent(UCharacterWeaponComponent* WeaponComp)
	{
		WeaponComponent = WeaponComp;
	}
	
	void AddArmorComponent(UCharacterArmorComponent* ArmorComp)
	{
		ArmorComponent = ArmorComp;
	}
	
	void AddInteractionComponent(UPawnInteractionComponent* InteractionComp)
	{
		InteractionComponent = InteractionComp;
	}
};

UCLASS()
class STALKER_API AStalkerPlayerController : public AGenPlayerController
{
	GENERATED_BODY()

public:
	AStalkerPlayerController();
	
	static FName InventoryManagerComponentName;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<const UInputMappingContext> CharacterMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input", DisplayName = "UI Mapping Context")
	TObjectPtr<const UInputMappingContext> UIMappingContext;

	virtual void ClientSetHUD_Implementation(TSubclassOf<AHUD> NewHUDClass) override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	FORCEINLINE UInventoryManagerComponent* GetInventoryManager() const { return InventoryManager; }

protected:
	TObjectPtr<class AGameHUD> GameHUD;
	
	TObjectPtr<AStalkerCharacter> Character;

	virtual void PostInitializeComponents() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void OnPawnChanged(APawn* InOldPawn, APawn* InNewPawn);
	
	void ChooseAndSetupPawn(APawn* InPawn);
	void ChooseAndClearPawn(APawn* InPawn);
	
	void SetupCharacter();
	void ClearCharacter();
	void ConnectCharacterHUD();
	void DisconnectCharacterHUD();

	void InitEssentialComponents();
	void UnInitEssentialComponents();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Manager", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryManagerComponent> InventoryManager;

	bool bIsCharacterInitialized = false;
};
