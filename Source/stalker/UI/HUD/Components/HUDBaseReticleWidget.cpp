// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDBaseReticleWidget.h"
#include "CharacterWeaponComponent.h"
#include "CircumferenceMarkerWidget.h"
#include "Weapons/WeaponObject.h"

void UHUDBaseReticleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	CircumferenceMarker->SetRadius(ComputeMaxScreenspaceSpreadRadius());
}

void UHUDBaseReticleWidget::SetupReticleWidget(UCharacterWeaponComponent* WeaponComponent)
{
	OwnWeaponComponent = WeaponComponent;
	if (OwnWeaponComponent.IsValid())
	{
		
	}
}

float UHUDBaseReticleWidget::ComputeMaxScreenspaceSpreadRadius() const
{
	const float LongShotDistance = 10000.f;

	// TODO: CameraManager
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		const float SpreadRadiusRads = FMath::DegreesToRadians(ComputeSpreadAngle() * 0.5f);
		const float SpreadRadiusAtDistance = FMath::Tan(SpreadRadiusRads) * LongShotDistance;

		FVector CamPos;
		FRotator CamOrient;
		PC->GetPlayerViewPoint(CamPos, CamOrient);

		FVector CamForwDir = CamOrient.RotateVector(FVector::ForwardVector);
		FVector CamUpDir   = CamOrient.RotateVector(FVector::UpVector);
		FVector OffsetTargetAtDistance = CamPos + CamForwDir * LongShotDistance + CamUpDir * SpreadRadiusAtDistance;

		FVector2D OffsetTargetInScreenspace;
		if (PC->ProjectWorldLocationToScreen(OffsetTargetAtDistance, OffsetTargetInScreenspace, true))
		{
			int32 ViewportSizeX(0), ViewportSizeY(0);
			PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

			const FVector2D ScreenSpaceCenter(FVector::FReal(ViewportSizeX) * 0.5f, FVector::FReal(ViewportSizeY) * 0.5f);
			return (OffsetTargetInScreenspace - ScreenSpaceCenter).Length();
		}
	}
	
	return 0.0f;
}

float UHUDBaseReticleWidget::ComputeSpreadAngle() const
{
	if (OwnWeaponComponent.IsValid())
	{
		if (const UWeaponObject* WeaponObject = Cast<const UWeaponObject>(OwnWeaponComponent->GetItemObjectAtRightHand()))
		{
			return WeaponObject->GetSpreadAngle();
		}
	}
	return 0.0f;
}
