// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "HUDBaseReticleWidget.generated.h"

class UCharacterWeaponComponent;

UCLASS()
class STALKER_API UHUDBaseReticleWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetupReticleWidget(UCharacterWeaponComponent* WeaponComponent);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCircumferenceMarker> CircumferenceMarker;
	
private:
	TWeakObjectPtr<UCharacterWeaponComponent> OwnWeaponComponent;
	
	float ComputeMaxScreenspaceSpreadRadius() const;
	float ComputeSpreadAngle() const;
};
