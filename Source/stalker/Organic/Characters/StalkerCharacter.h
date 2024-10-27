// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

class UCharacterArmorComponent;

UCLASS(Blueprintable, BlueprintType)
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	static FName ArmorComponentName;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	virtual bool CheckFireAbility();
	virtual bool CheckAimingAbility();
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
	/* TODO:
	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	TObjectPtr<class UOrganicAttributeSet> OrganicAttributeSet;
	*/

	void OnAimingStart();
	void OnAimingStop();
	void OnOverlayChanged(ECharacterOverlayState NewOverlay);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterArmorComponent> ArmorComponent;
	
	bool bIsStalkerInitialized = false;
};
