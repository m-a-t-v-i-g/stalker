// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

class UWeaponComponent;
class UItemObject;

UCLASS()
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PossessedBy(AController* NewController) override;

	static FName ArmorComponentName;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterArmorComponent> ArmorComponent;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	FORCEINLINE UCharacterArmorComponent* GetArmorComponent() const { return ArmorComponent; }

	template <class T>
	T* GetArmorComponent() const
	{
		return Cast<T>(GetArmorComponent());
	}

protected:
	void OnWeaponOverlayChanged(ECharacterOverlayState NewOverlay);
};
