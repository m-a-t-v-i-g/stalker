// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Library/CharacterLibrary.h"
#include "Organic/BaseOrganic.h"
#include "BaseCharacter.generated.h"

class UStalkerCharacterMovementComponent;
class UInventoryComponent;
class UWeaponComponent;

UCLASS(Abstract, NotBlueprintable, NotBlueprintType)
class STALKER_API ABaseCharacter : public ABaseOrganic
{
	GENERATED_BODY()

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	static FName CharacterMovementName;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;
	
	void SetMovementAction(ECharacterMovementAction NewAction, bool bForce = false);
	FORCEINLINE ECharacterMovementAction GetMovementAction() const { return MovementAction.Action; }
	
	virtual void OnMovementActionChanged(ECharacterMovementAction PreviousAction);

	void SetOverlayState(ECharacterOverlayState NewState, bool bForce = false);
	FORCEINLINE ECharacterOverlayState GetOverlayState() const { return OverlayState.State; }

	virtual void OnOverlayStateChanged(ECharacterOverlayState PreviousState);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UStalkerCharacterMovementComponent* GetCharacterMovement() const { return CharacterMovementComponent; }
	
	template <class T>
	T* GetCharacterMovement() const
	{
		return Cast<T>(GetCharacterMovement());
	}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
	TObjectPtr<class UMovementModelConfig> DefaultMovementModel;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Character|Movement")
	FCharacterMovementAction MovementAction = ECharacterMovementAction::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Character|State")
	FCharacterOverlayState OverlayState = ECharacterOverlayState::Default;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStalkerCharacterMovementComponent> CharacterMovementComponent;
};
