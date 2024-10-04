// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

class UGameplayAbility;
class UItemObject;
class UWeaponComponent;
class AStalkerPlayerController;

UCLASS()
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

	static FName ArmorComponentName;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterArmorComponent> ArmorComponent;
	
	bool bIsStalkerInitialized = false;
	
public:
	virtual void SetupCharacterLocally(AController* NewController);
	
protected:
	void OnAimingStart();
	void OnAimingStop();
	void OnOverlayChanged(ECharacterOverlayState NewOverlay);

public:
	virtual bool CheckReloadAbility();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UCharacterArmorComponent* GetArmorComponent() const { return ArmorComponent; }

	template <class T>
	T* GetArmorComponent() const
	{
		return Cast<T>(GetArmorComponent());
	}
};
