// Copyright 2022 Dominik Lips. All Rights Reserved.

#include "Speedometer.h"

bool USpeedometer::Initialize()
{
  if (Super::Initialize())
  {
    SetNumberFormattingOptions();
    return true;
  }
  return false;
}

void USpeedometer::SetNumberFormattingOptions()
{
  NumberFormat.SetAlwaysSign(false);
  NumberFormat.SetUseGrouping(false);
  NumberFormat.SetRoundingMode(ERoundingMode::HalfFromZero);
  NumberFormat.SetMinimumIntegralDigits(1);
  NumberFormat.SetMaximumIntegralDigits(10);
  NumberFormat.SetMinimumFractionalDigits(0);
  NumberFormat.SetMaximumFractionalDigits(0);
}

void USpeedometer::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
  Super::NativeTick(MyGeometry, InDeltaTime);

  Value_Speedometer = 0.f;
  if (const auto MovementComponent = GetLocalPawnMovementComponent())
  {
    const FVector& Velocity = MovementComponent->Velocity;
    Value_Speedometer = bIncludeVelocityZ ? Velocity.Size() : Velocity.Size2D();
  }

  if (Text_Speedometer) Text_Speedometer->SetText(FText::AsNumber(Value_Speedometer, &NumberFormat));
}

UPawnMovementComponent* USpeedometer::GetLocalPawnMovementComponent() const
{
  if (const auto World = GetWorld())
  {
    for (auto Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
      const APlayerController* PlayerController = Iterator->Get();
      if (PlayerController && PlayerController->IsLocalController())
      {
        if (const APawn* LocalPawn = PlayerController->GetPawn())
        {
          return LocalPawn->FindComponentByClass<UPawnMovementComponent>();
        }
        break;
      }
    }
  }
  return nullptr;
}
