// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InteractorInterface.h"
#include "StalkerCharacter.generated.h"

class UAbilitySet;
class UCharacterArmorComponent;

UCLASS(Blueprintable, BlueprintType)
class STALKER_API AStalkerCharacter : public ABaseCharacter, public IInteractorInterface
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	static FName ArmorComponentName;
	
	virtual void PostInitializeComponents() override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	virtual bool CheckReloadAbility();

	virtual void SetupCharacterLocally(AController* NewController);
	
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
	
	void OnAimingStart();
	void OnAimingStop();
	void OnOverlayChanged(ECharacterOverlayState NewOverlay);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterArmorComponent> ArmorComponent;
	
	bool bIsStalkerInitialized = false;
};
