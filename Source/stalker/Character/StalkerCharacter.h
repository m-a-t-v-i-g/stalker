// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseOrganic.h"
#include "InteractorInterface.h"
#include "StalkerCharacter.generated.h"

class UAbilitySet;
class UStalkerCharacterMovementComponent;
class UCharacterInventoryComponent;
class UCharacterWeaponComponent;
class UCharacterStateComponent;
class UCharacterArmorComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogCharacter, Log, All);

UCLASS(Abstract)
class STALKER_API AStalkerCharacter : public ABaseOrganic, public IInteractorInterface
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);
	
	static FName CharacterMovementName;
	static FName InventoryComponentName;
	static FName WeaponComponentName;
	static FName StateComponentName;
	static FName ArmorComponentName;
	
	virtual void PostInitializeComponents() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	virtual void InitCharacterComponents();
	virtual void SetupCharacterLocally();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UStalkerCharacterMovementComponent* GetCharacterMovement() const { return CharacterMovementComponent; }
	
	template <class T>
	T* GetCharacterMovement() const
	{
		return Cast<T>(GetCharacterMovement());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UCharacterInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	template <class T>
	T* GetInventoryComponent() const
	{
		return Cast<T>(GetInventoryComponent());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UCharacterWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	template <class T>
	T* GetWeaponComponent() const
	{
		return Cast<T>(GetWeaponComponent());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UCharacterStateComponent* GetStateComponent() const { return StateComponent; }

	template <class T>
	T* GetStateComponent() const
	{
		return Cast<T>(GetStateComponent());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UCharacterArmorComponent* GetArmorComponent() const { return ArmorComponent; }

	template <class T>
	T* GetArmorComponent() const
	{
		return Cast<T>(GetArmorComponent());
	}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TObjectPtr<UAbilitySet> AbilitySet;

	virtual void SetCharacterData();
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStalkerCharacterMovementComponent> CharacterMovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterInventoryComponent> InventoryComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterWeaponComponent> WeaponComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterStateComponent> StateComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterArmorComponent> ArmorComponent;
};
