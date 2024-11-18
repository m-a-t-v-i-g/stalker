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

DECLARE_MULTICAST_DELEGATE(FOnPlayerToggleInventorySignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerToggleSlotSignature, uint8);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLootContainerSignature, UInventoryComponent*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerPickUpItemSignature, UItemObject*);

UCLASS()
class STALKER_API APlayerCharacter : public AStalkerCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	static FName InteractionComponentName;

	FOnPlayerToggleInventorySignature OnPlayerToggleInventory;

	FOnPlayerToggleSlotSignature OnPlayerToggleSlot;
	
	FOnPlayerLootContainerSignature OnContainerInteraction;
	
	FOnPlayerPickUpItemSignature OnItemInteraction;
	
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

	virtual bool ContainerInteract(UInventoryComponent* TargetInventory) override;
	
	UFUNCTION(Client, Reliable)
	void ClientContainerInteract(UInventoryComponent* TargetInventory);
	
	virtual bool ItemInteract(UItemObject* ItemObject) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void SetupCharacterLocally() override;
	
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
	TObjectPtr<UInteractionComponent> InteractionComponent;
};
